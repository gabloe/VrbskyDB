
#ifndef OS_FILE_H_
#define OS_FILE_H_

#include <string>

#include "Constants.h"
#include "FileSystem.h"

namespace os {

        class File {

            private:

                FileSystem *fs;

                // Properties of file/data
                std::string name;
                FileStatus status;
                uint64_t size;

                // Position information
                uint64_t first;    // First block
                uint64_t last;    // First block
                uint64_t block;     // Current block
                uint64_t position;  // Current position in the block

                File() {} 
            public:
                File( const File &other ) {

                    fs = other.fs;

                    name = other.name;
                    status = other.status;
                    size = other.size;

                    first = other.first;
                    last = other.last;
                    block = other.block;
                    position = other.position;
                }


                std::string getFilename() const {
                    return name;
                }

                uint64_t read( uint64_t length , char* buffer );
                uint64_t write( uint64_t length , const char* buffer );
                uint64_t insert( uint64_t length , const char* buffer );
                uint64_t remove( uint64_t length );

                uint64_t read( uint64_t start , uint64_t length , char* buffer );
                uint64_t write( uint64_t start , uint64_t length , const char* buffer );
                uint64_t insert( uint64_t start , uint64_t length , const char* buffer );
                uint64_t remove( uint64_t start , uint64_t length );

                bool unlink();
                bool rename( const std::string newName );
                FileStatus getStatus() const;

                void close();

                friend class FileSystem;
        };
}


#endif
