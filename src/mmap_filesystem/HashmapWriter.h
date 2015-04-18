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
		HashmapWriter(File &file_, Filesystem &fs_): fs(fs_), file(file_) {}
		uint64_t write_buffer(std::map<std::string, T> data, char *&buffer) {
			uint64_t size = BLOCK_SIZE;
			uint64_t pos = 0;
			buffer = (char*)malloc(size);
			for (auto it = data.begin(); it != data.end(); ++it) {
				std::string key = it->first;
				uint64_t key_size = key.size();
				T val = it->second;

				growBuffer(buffer, pos, size);
				memcpy(buffer+pos, &key_size, sizeof(uint64_t));
				pos += sizeof(uint64_t);

				growBuffer(buffer, pos, size);
				memcpy(buffer+pos, key.c_str(), key_size);
				pos += key_size;

				growBuffer(buffer, pos, size);
				memcpy(buffer+pos, &val, sizeof(T));
				pos += sizeof(T);
			}
			buffer = (char*)realloc(buffer, pos);
			return pos;
		}
		uint64_t write(std::map<std::string, T> data) {
			char *buffer;
			uint64_t size = write_buffer(data, buffer);			
			fs.write(&file,buffer,size);
			return size;
		}
	private:
		Filesystem &fs;
		File &file;
		void growBuffer(char *&buffer, uint64_t pos, uint64_t &size) {
			if (pos > size) {
				size *= 2;
				buffer = (char*)realloc(buffer,size);
			}
		}
	};
}

#endif
