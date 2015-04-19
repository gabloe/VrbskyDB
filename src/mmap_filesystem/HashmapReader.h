#ifndef _HASHMAP_READER_H_
#define _HASHMAP_READER_H_

#include <map>
#include <string.h>
#include "Filesystem.h"

namespace Storage {
	template <typename T>
	class HashmapReader {
	public:
		HashmapReader(File &file_, Filesystem &fs_): fs(fs_), file(file_) {}
		std::map<std::string, T> read_buffer(char *buffer, uint64_t size) {
			std::map<std::string, T> result;
			uint64_t pos = 0;
			char *key_tmp = NULL;
			while (pos < size) {
				uint64_t key_size;
				memcpy(&key_size, buffer + pos, sizeof(uint64_t));
				pos += sizeof(uint64_t);

				key_tmp = (char*)realloc(key_tmp, key_size);
				memcpy(key_tmp, buffer + pos, key_size);
				std::string key(key_tmp, key_size);
				pos += key_size;

				T val;
				memcpy(&val, buffer + pos, sizeof(T));
				pos += sizeof(T);

				result[key] = val;
			}
			free(key_tmp);
			return result;
		}
		std::map<std::string, T> read() {
			std::map<std::string, T> result;
			uint64_t size = file.size;
			char *buffer = fs.read(&file);
			result = read_buffer(buffer, size);
			return result;
		}
	private:
		Filesystem &fs;
		File &file;
	};
}

#endif
