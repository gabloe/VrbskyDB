#ifndef _HASHMAP_WRITER_H_
#define _HASHMAP_WRITER_H_

#include <string>
#include <cstring>
#include <map>

#include "../mmap_filesystem/Filesystem.h"
#include "../mmap_filesystem/Util.h"

namespace Storage {
    template <typename T>
        class HashmapWriter {
            public:
                HashmapWriter(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
                char *write_buffer(std::map<std::string, T> data, uint64_t *size) {

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
                        if (pos + (key_size + sizeof(uint64_t) + value_size) > buf_size) {
                            buf_size += key_size + sizeof(uint64_t) + value_size;
                            buf_size *= 2;
                            buffer = (char*)realloc(buffer, buf_size);
                        }

                        // Output key
                        Write64( buffer , pos , key_size );
                        WriteString( buffer , pos , key );

                        // Output value
                        Write64( buffer , pos , value_size );
                        WriteRaw( buffer , pos , Type<T>::Bytes(val) , value_size );

                    }
                    buffer = (char*)realloc(buffer, pos);
                    *size = pos;
                    return buffer;
                }
                uint64_t write(std::map<std::string, T> data) {
                    char *buffer;
                    uint64_t size;
                    buffer = write_buffer(data, &size);			
                    fs->write(&file,buffer,size);
                    return size;
                }
            private:
                Filesystem *fs;
                File &file;
        };
}

#endif
