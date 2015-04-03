
#ifndef OS_FILE_H_
#define OS_FILE_H_

#include <string>

#include "Constants.h"
#include "FileSystem.h"

namespace os {

        class File {


            public:
                FileSystem *fs;

                // Properties of file/data
                std::string name;
                FileStatus status;
                uint64_t size;

                // Position information
                uint64_t start;     // First block
                uint64_t end;       // Last block
                uint64_t current;   // Current block
                uint64_t position;  // Position from first element in current block
                uint64_t metadata;  // Position of metadata (byte offset)

                File(): size(0), position(0), start(0), end(0), current(0) {} 

                File( const File &other ) {

                    fs = other.fs;

                    name    = other.name;
                    status  = other.status;
                    size    = other.size;

                    start       = other.start;
                    end         = other.end;
                    current     = other.current;
                    position    = other.position;
                    metadata    = other.metadata;
                }

                // Return the current filename seen by this file descriptor
                std::string getFilename() const {
                    return name;
                }

                // Removes the file from the filesystem
                bool unlink();

                // Rename the file
                bool rename( const std::string newName );

                // Get the current status
                FileStatus getStatus() const;

                friend class FileSystem;
                friend class FileReader;
                friend class FileWriter;
        };
}


#endif
