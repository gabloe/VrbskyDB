#ifndef _HASHMAP_WRITER_H_
#define _HASHMAP_WRITER_H_

#include <string>
#include <cstring>

#include "../storage/HerpHash.h"
#include "../mmap_filesystem/Filesystem.h"
#include "../utils/Util.h"
#include "../assert/Assert.h"

namespace Storage {
    template <typename T,uint64_t Buckets = 1024>
        class HerpmapWriter {
            public:
                HerpmapWriter(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
                char *write_buffer(Storage::HerpHash<std::string, T,Buckets> data, uint64_t *size) {

                    uint64_t buf_size = BLOCK_SIZE;
                    uint64_t pos = 0;
                    char *buffer = (char*)malloc(buf_size);

                    for (auto it = data.begin(); it != data.end(); ++it) {

                        // data
                        std::string key = it->first;
                        T val = it->second;

                        // size
                        uint64_t key_size = key.size();
                        uint64_t value_size = Type<T>::Size(val);

                        // Re-grow to fit
                        if (pos + (key_size + 2*sizeof(uint64_t) + value_size) > buf_size) {
                            buf_size += key_size + 2*sizeof(uint64_t) + value_size;
                            buf_size *= 2;
                            Assert( "New  size is 0?" , buf_size > 0 );
                            buffer = (char*)realloc(buffer, buf_size);
                        }

                        // Output key
                        Write64( buffer , pos , key_size );
                        WriteString( buffer , pos , key );

                        // Output value
                        Write64( buffer , pos , value_size );
                        const char *data = Type<T>::Bytes(val);
                        WriteRaw( buffer , pos , data , value_size );
                        delete[] data;

                    }
                    buffer = (char*)realloc(buffer, pos);
                    *size = pos;
                    return buffer;
                }
                uint64_t write(Storage::HerpHash<std::string, T,Buckets> data) {
                    char *buffer;
                    uint64_t size;
                    buffer = write_buffer(data, &size);			
                    fs->write(&file,buffer,size);
		    free(buffer);
                    return size;
                }
            private:
                Filesystem *fs;
                File &file;
        };
}

#endif
