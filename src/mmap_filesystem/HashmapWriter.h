#ifndef _HASHMAP_WRITER_H_
#define _HASHMAP_WRITER_H_

#include <string>
#include <string.h>
#include <map>
#include "Filesystem.h"

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
				uint64_t key_size = key.size();
				T val = it->second;

                // Re-grow to fit
				if (pos + (key_size + sizeof(uint64_t) + sizeof(T)) > buf_size) {
                    buf_size += key_size + sizeof(uint64_t) + sizeof(T);
					buf_size *= 2;
					buffer = (char*)realloc(buffer, buf_size);
				}

                // Copy over
				memcpy(buffer+pos, &key_size, sizeof(uint64_t));
				pos += sizeof(uint64_t);

				memcpy(buffer+pos, key.c_str(), key_size);
				pos += key_size;

				memcpy(buffer+pos, &val, sizeof(T));
				pos += sizeof(T);
			}
            // TODO: Shrink?
			//buffer = (char*)realloc(buffer, pos);
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
