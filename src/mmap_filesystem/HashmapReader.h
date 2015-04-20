#ifndef _HASHMAP_READER_H_
#define _HASHMAP_READER_H_

#include <map>
#include <string.h>
#include "Filesystem.h"

namespace Storage {
	template <typename T>
	class HashmapReader {
	public:
		HashmapReader(File &file_, Filesystem *fs_): fs(fs_), file(file_) {}
		std::map<std::string, T> read_buffer(char *buffer, uint64_t offset, uint64_t size) {
			std::map<std::string, T> result;
			uint64_t pos = offset;
			while (pos < size) {
				uint64_t key_size;
				memcpy(&key_size, buffer + pos, sizeof(uint64_t));
				pos += sizeof(uint64_t);

				char *key_tmp = (char*)malloc(key_size);
				strncpy(key_tmp, buffer + pos, key_size);
				std::string key(key_tmp, key_size);
				free(key_tmp);
				pos += key_size;

				T val;
				memcpy(&val, buffer + pos, sizeof(T));
				pos += sizeof(T);
		
                		std::cout << "Reading file: " << key << std::endl;
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
