#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <string>
#include <sys/stat.h>
#include <sys/mman.h>
//#include <map>
#include "../storage/HerpHash.h"
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <mutex>

const uint64_t BLOCK_SIZE  = 64;
const uint64_t BLOCKS_PER_PAGE = 1024;
const uint64_t BLOCK_SIZE_ACTUAL = 3 * sizeof(uint64_t) + BLOCK_SIZE;
const uint64_t PAGESIZE = BLOCK_SIZE_ACTUAL * BLOCKS_PER_PAGE;

#ifndef UNUSED
#define UNUSED(X)
#endif

enum lock_t {
	WRITE,
	READ
};

struct Block {
	uint64_t id;
	uint64_t used_space;
	uint64_t next;
	char buffer[BLOCK_SIZE];
};

#ifdef __APPLE__
inline int bsd_fallocate(int, off_t, off_t);
inline void *bsd_mremap(int, void *, size_t, size_t, int);
inline int bsd_fallocate(int fd, off_t offset, off_t size) {
	fstore_t fst;
	struct stat sb;
	int ret;
	uint64_t newSize = offset + size;

	fst.fst_flags = F_ALLOCATECONTIG; // could add F_ALLOCATEALL
	fst.fst_posmode = F_PEOFPOSMODE; // allocate from EOF (0)
	fst.fst_offset = 0; // offset relative to the EOF
	fst.fst_length = newSize;
	ret = fcntl(fd, F_PREALLOCATE, &fst);
	if(ret == -1) { // read fcntl docs - must test against -1
		std::cerr << "Could not pre-allocate!" << std::endl;
		exit(0);
	}

	ret = fstat(fd, &sb);
	if(ret != 0) {
		std::cerr << "Could not stat file!" << std::endl;
		exit(0);
	}

	ret = ftruncate(fd, newSize); // NOT fst.fst_bytesAllocated!
	if(ret != 0) {
		std::cerr << "Could not expand file!" << std::endl;
		exit(0);
	}

	return ret; 
}
inline void *bsd_mremap(int fd, void *old_address, size_t old_size, size_t new_size, int flags) {
	(void)(flags);
	munmap(old_address, old_size);
	return mmap(old_address, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
}
#define posix_fallocate bsd_fallocate
#define t_mremap bsd_mremap
#define MREMAP_MAYMOVE 0
#else
inline void *linux_mremap(int UNUSED(fd), void *old_address, size_t old_size, size_t new_size, int flags) {
	return mremap(old_address, old_size, new_size, flags);
}
#define t_mremap linux_mremap 
#endif

struct File {
	std::string name;
	uint64_t block;
	uint64_t size;
	File() {}
	File(std::string name_, uint64_t block_, uint64_t size_): name(name_), block(block_), size(size_) {}
};

struct Metadata {
	uint64_t numFiles;
	uint64_t firstFree;
	File file;
	//std::map<std::string, uint64_t> files;
    Storage::HerpHash<std::string,uint64_t> files;
};

struct FSystem {
	uint64_t numPages;
	int fd;
	char *data;
};

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
		Filesystem(std::string);
		void shutdown();
		File open_file(const char*);
		File open_file(const std::string&);
		char *read(File*);
		void write(File*, const char*, uint64_t);
		bool deleteFile(File*);
		std::vector<std::string> getFilenames();
		//std::map<std::string, uint64_t> getFileMap();
	        Storage::HerpHash<std::string,uint64_t> getFileMap();
		void compact();
		uint64_t getNumPages();
		uint64_t getNumFiles();

		void Lock(lock_t, File);
		void Unlock(lock_t, File);

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
		void chainPage(uint64_t);
		void addToFreeList(uint64_t);
		std::mutex next_lock;
		std::map<std::string, std::mutex> read_locks;
		std::map<std::string, std::mutex> write_locks;
	};
}

#endif

