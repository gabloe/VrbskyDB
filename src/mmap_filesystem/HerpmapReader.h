#ifndef _HASHMAP_READER_H_
#define _HASHMAP_READER_H_

#include <string>

#include "../storage/HerpHash.h"
#include "../mmap_filesystem/Filesystem.h"
#include "../utils/Util.h"

namespace Storage {
    template <typename T,uint64_t Buckets=1024>
        class HerpmapReader {
            public:
                HerpmapReader(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
                Storage::HerpHash<std::string, T,Buckets> read_buffer(const char *buffer, uint64_t pos, uint64_t size) {
                    Storage::HerpHash<std::string,T,Buckets> result;
                    while (pos < size) {
                        Assert( "Position went to far", pos < size );

                        // key
                        uint64_t key_size = Read64( buffer , pos );
                        std::string key = ReadString( buffer , pos , key_size );

                        // Value
                        uint64_t value_size = Read64( buffer , pos );
                        T val = Type<T>::Create( buffer + pos , value_size );
                        pos += value_size;

                        // Save
                        result.put( key , val );
                    }
                    return result;
                }
                Storage::HerpHash<std::string,T,Buckets> read() {
                    Storage::HerpHash<std::string,T,Buckets> result;
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
