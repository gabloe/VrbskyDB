#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <string>
#include <sys/stat.h>
#include <map>

#define BLOCK_SIZE 32
#define BLOCKS_PER_PAGE 128
#define PAGESIZE BLOCK_SIZE * BLOCKS_PER_PAGE


struct Block {
	uint64_t id;
	uint64_t used_space;
	uint64_t next;
	char buffer[BLOCK_SIZE];
};

struct File {
	std::string name;
	uint64_t block;
	uint64_t size;
	File(std::string name_, uint64_t block_, uint64_t size_): name(name_), block(block_), size(size_) {}
};

struct Metadata {
	uint64_t numPages;
	uint64_t numFiles;
	uint64_t firstFree;
	int fd;
	char *data;
	std::map<std::string, uint64_t> files;
};

struct FSystem {
	uint64_t numPages;
	int fd;
	char *data;
};

const char HEADER[] = { 0x0D, 0x0E, 0x0A, 0x0D, 0x0B, 0x0E, 0x0E, 0x0F };

inline bool file_exists(std::string fname) {
	struct stat buf;
	if (stat(fname.c_str(), &buf) == -1) {
		return false;
	}
	return true;
}

namespace Storage {
	class Filesystem {
	public:
		Filesystem(std::string, std::string);
		void shutdown();
		File load(std::string);
		char *read(File*);
		void write(File*, const char*, uint64_t);

	protected:
		Metadata metadata;
		FSystem filesystem;

		int fileSystemSize;
		int metaDataSize;

		std::string meta_fname;
		std::string data_fname;

		uint64_t getBlock();
		File createNewFile(std::string);
		void initFilesystem(bool);
		void readMetadata();
		void writeMetadata();
		void initMetadata();
		Block loadBlock(uint64_t);
		void writeBlock(Block);
		void growFilesystem();
		void growMetadata();
		uint64_t calculateSize(Block);
	};
}

#endif

