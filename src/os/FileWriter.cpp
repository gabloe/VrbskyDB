
#include "FileSystem.h"
#include "File.h"
#include "FileWriter.h"

namespace os {

    FileWriter::FileWriter( File f): file(f) {
        /* Blank on purpose */
    }

    uint64_t FileWriter::write( uint64_t length , const char* buffer ) {
        return file.fs->write( file , length , buffer );
    }

    void FileWriter::seek( uint64_t position , FilePosition from ) {
        // TODO:    performance efficiency, try to change 
        //          current to the correct current instead
        //          of start
        Block b;
        switch( from ) {
            case BEG:
                file.position = position;
                file.current = file.start;
                break;
            case END:
                file.position = file.size - position;
                file.current = file.start;
                break;
            case CUR:
                file.position += position;
                file.current = file.start;
                break;
        };
    }

    void FileWriter::close() {
    }
};
