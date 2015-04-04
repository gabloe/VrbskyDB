
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
                // Size information
                uint64_t disk_usage;    // Bytes on disk (excluding meta-data)
                uint64_t size;          // Bytes used
                uint64_t num_blocks;    // Number of blocks allocated (can be computed)

                // Position information
                uint64_t start;     // First block
                uint64_t end;       // Last block
                uint64_t metadata;  // Position of metadata (byte offset)

                // Live state
                uint64_t current;   // Current block
                uint64_t b_position;// Position in current block

                uint64_t position;  // Number of used bytes since first block
                uint64_t b_position;// Number of allocated bytes since first block

                File(): size(0), position(0), start(0), end(0), current(0) , b_position(0) {} 

                File( const File &other ) {

                    fs = other.fs;

                    name    = other.name;
                    status  = other.status;
                    size    = other.size;

                    // Saved meta-data
                    start       = other.start;
                    end         = other.end;
                    metadata    = other.metadata;

                    // State
                    current     = other.current;
                    position    = other.position;
                    b_position  = other.b_position;
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
