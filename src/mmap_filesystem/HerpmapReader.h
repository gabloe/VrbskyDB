#ifndef _HASHMAP_READER_H_
#define _HASHMAP_READER_H_

#include <string>

#include "../storage/HerpHash.h"
#include "../mmap_filesystem/Filesystem.h"
#include "../utils/Util.h"

namespace Storage {
    template <typename T>
        class HerpmapReader {
            public:
                HerpmapReader(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
                Storage::HerpHash<std::string, T> read_buffer(char *buffer, uint64_t offset, uint64_t size) {
                    Storage::HerpHash<std::string,T> result;
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
                        std::cout << "Reading "  << key << ": " << val << std::endl;
                        result.put( key , val );
                    }
                    return result;
                }
                Storage::HerpHash<std::string,T> read() {
                    Storage::HerpHash<std::string,T> result;
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
