
#include "FileReader.h"

#include "FileSystem.h"
#include "File.h"


namespace os {

    FileReader::FileReader(File& f) : file(f) {
        /* Empty Constructor */
    }

    // Read all of the data in this file and return it.
    std::string FileReader::readAll() {
        char buff[file.size];
        uint64_t size = file.fs->read( file, file.size, buff );
        assert( size != 0 );
        return std::string(buff, file.size);
    }

    uint64_t FileReader::read( uint64_t length , char* buffer ) {
        // TODO:    As of now the current is always start
        //          In the future it would be nice to
        //          not linearly search the entire file

        return file.fs->read( file , length , buffer );
    }

    // Move the file pointer to a new position
    void FileReader::seek( uint64_t position , FilePosition pos ){
        // TODO:    performance efficiency, try to change 
        //          current to the correct current instead
        //          of start
        Block b;
        switch( pos ) {
            case BEG:
                {
                    file.position = position;
                    file.current = file.start;
                    break;
                }
            case END:
                {
                    file.position = file.size - position;
                    file.current = file.start;
                    break;
                }
            case CUR:
                {
                    file.position += position;
                    break;
                }
        };
    }

    uint64_t FileReader::tell() {
        return file.position;
    }

    void FileReader::close() {
        file.fs->close(file.name);
    }

};
