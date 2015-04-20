#ifndef _HASHMAP_READER_H_
#define _HASHMAP_READER_H_

#include <map>
#include <string.h>

#include "../mmap_filesystem/Filesystem.h"
#include "../mmap_filesystem/Util.h"

namespace Storage {
    template <typename T>
        class HashmapReader {
            public:
                HashmapReader(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
                std::map<std::string, T> read_buffer(char *buffer, uint64_t offset, uint64_t size) {
                    std::map<std::string, T> result;
                    uint64_t pos = offset;
                    while (pos < size) {
                        // key
                        uint64_t key_size = Read64( buffer , pos );
                        std::string key = ReadString( buffer , pos , key_size );

                        // Value
                        uint64_t value_size = Read64( buffer , pos );
                        T val = Type<T>::Create( buffer + pos , value_size );
                        pos += value_size;

                        // Save
                        result[key] = val;
                    }
                    return result;
                }
                std::map<std::string, T> read() {
                    std::map<std::string, T> result;
                    uint64_t size = file.size;
                    char *buffer = fs->read(&file);
                    result = read_buffer(buffer, 0, size);
                    free(buffer);
                    return result;
                }
            private:
                Filesystem *fs;
                File &file;
        };
}

#endif
