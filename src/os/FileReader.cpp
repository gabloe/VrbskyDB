
#include "../os/FileReader.h"

#include "../os/FileSystem.h"
#include "../os/File.h"


namespace os {

    FileReader::FileReader(File& f) : file(f) {
        /* Empty Constructor */
    }

    // Read all of the data in this file and return it.
    char* FileReader::readAll() {
        char *buff = new char[file.size];
        uint64_t size = file.fs->read( file, file.size, buff );
        assert( file.size != 0 );
        return buff;
    }

    uint64_t FileReader::read( uint64_t length , char* buffer ) {
        // TODO:    As of now the current is always start
        //          In the future it would be nice to
        //          not linearly search the entire file

        return file.fs->read( file , length , buffer );
    }

    // Move the file pointer to a new position
    void FileReader::seek( int64_t position , FilePosition from ){
        // TODO:    Improve performance by moving from my current position
        switch( from ) {
            case BEG:
                file.current = file.start;
                file.position = 0;
                file.block_position = 0;
                file.disk_position = 0;
                break;
            case END:
                file.current = file.start;
                file.position = 0;
                file.block_position = 0;
                file.disk_position = 0;
                position = file.size - position;
                break;
            case CUR:
                if( position < 0 ) {
                    position = position + file.position;
                    file.current = file.start;
                    file.block_position = 0;
                    file.disk_position = 0;
                }
                break;
        };

        // Assertion:   The files current position is not
        //              after the requested position
        assert( file.position <= position );

        if( position < 0 ) {
            throw std::runtime_error( "Invalid file position" );
        }

        uint64_t pos = position;

        uint64_t current = file.current;

        // While we have more disk space and we have not reached the position 
        while( file.disk_position < file.disk_usage && file.position < position ) {
            // Load block
            Block b;
            file.fs->load( current , &b );

            // Minimum of remaining bytes in current block
            // and distance to position requested
            uint64_t move = std::min( b.length - file.block_position , pos );

            // Update
            if( pos > move ) { // About to go to next block
                file.disk_position += Block_Size;
                file.block_position = 0;
            }else {
                file.block_position += move;
                file.disk_position += move;
            }
            file.position += move;
            pos -= move;

            // Get next
            current = b.next;
        }

        assert( file.position == position );

        // Out of the file
        if( file.position < position ) {
            file.current = 0;
            file.position += position;
            file.disk_position += round<Block_Size>(position);
            file.block_position = position % Block_Size;
        }
    }

    uint64_t FileReader::tell() {
        return file.position;
    }

    void FileReader::close() {
        file.fs->close(file.name);
    }

};
