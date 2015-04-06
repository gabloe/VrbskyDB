
#ifndef OS_FILE_H_
#define OS_FILE_H_

#include <string>

#include "Constants.h"
#include "FileSystem.h"

namespace os {

        class File {


            public:

                // Properties of file/data
                std::string name;

                // Size information
                uint64_t disk_usage;    // Bytes on disk (excluding meta-data)
                uint64_t size;          // Bytes used
                uint64_t num_blocks;    // Number of blocks allocated (can be computed)

                // Position information
                uint64_t start;         // First block
                uint64_t end;           // Last block
                uint64_t metadata;      // Position of metadata (byte offset)

                // Live state
                FileStatus status;      // The current status of me
                FileSystem *fs;         // Filesytem that I am from
                uint64_t current;       // Current block

                uint64_t position;      // Offset in written bytes
                uint64_t block_position;// Offset in block
                uint64_t disk_position; // Offset in allocated bytes

                File() {
                    disk_usage = size = num_blocks = start = end = 0;
                    metadata = current = position = disk_position = block_position = 0;
                } 

                File( const File &other) = delete;

                // Return the current filename seen by this file descriptor
                std::string getFilename() const {
                    return name;
                }

                uint64_t length(void) {
                    return this->size;
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
