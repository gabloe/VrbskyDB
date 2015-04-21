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

                        std::cout << "Key has size " << key_size << std::endl;
                        for( int i = 0 ; i < key_size ; ++i ) {
                            std::cout << buffer[pos + i];
                        }std::cout << std::endl;

                        std::string key(buffer + pos , key_size );// = ReadString( buffer , pos , key_size );
                        pos += key_size;

                        // Value
                        uint64_t value_size = Read64( buffer , pos );
                        std::cout << "Value has size " << key_size << std::endl;
                        for( int i = 0 ; i < value_size ; ++i ) {
                            std::cout << buffer[pos + i];
                        }std::cout << std::endl;
                        T val = Type<T>::Create( buffer + pos , value_size );
                        pos += value_size;

                        // Save
                        result.put( key , val );
                    }
                    return result;
                }
                Storage::HerpHash<std::string,T> read() {
                    Storage::HerpHash<std::string,T> result;
                    char *buffer = fs->read(&file);
                    result = read_buffer(buffer, 0, file.size);
                    free(buffer);
                    return result;
                }
            private:
                Filesystem *fs;
                File &file;
        };
}

#endif
