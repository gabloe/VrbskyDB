
#include <exception>
#include <cassert>
#include <algorithm>

#include "FileSystem.h"
#include "File.h"
#include "FileWriter.h"

namespace os {

    FileWriter::FileWriter(File& f): file(f) {
        /* Blank on purpose */
    }

    uint64_t FileWriter::write( uint64_t length , const char* buffer ) {
        return file.fs->write( file , length , buffer );
    }

    //
    //  If the position occurs before my current position then I
    //  move back to the beginning and start from there.  Other
    //  wise I just start from where I am at and move forward
    //
    void FileWriter::seek( int64_t position , FilePosition from ) {
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
            Block b = file.fs->load( current );

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

    uint64_t FileWriter::tell() {
        return file.position;
    }

    void FileWriter::close() {
    }
};
