#include <iostream>
#include <string.h>
#include <fstream>
#include "Filesystem.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Need filename." << std::endl;
		exit(1);
	}

	std::string fname(argv[1], strlen(argv[1]));
	std::fstream in(fname, std::fstream::in | std::fstream::binary);
	uint64_t bid = 0;
	uint64_t usedSpace = 0;
	uint64_t next = 0;
	uint64_t numPages;
	uint64_t numFiles;
	uint64_t firstFree;
	char data[BLOCK_SIZE];
	in.read(reinterpret_cast<char*>(&bid), sizeof(uint64_t));
	in.read(reinterpret_cast<char*>(&usedSpace), sizeof(uint64_t));
	in.read(reinterpret_cast<char*>(&next), sizeof(uint64_t));
	in.read(data, BLOCK_SIZE);
	memcpy(&numPages, data, sizeof(uint64_t));
	memcpy(&numFiles, data + sizeof(uint64_t), sizeof(uint64_t));
	memcpy(&firstFree, data + 2*sizeof(uint64_t), sizeof(uint64_t));
	std::cout << "Metadata" << std::endl;
	std::cout << "\tBlock ID: " << bid << std::endl;
	std::cout << "\tUsed space: " << usedSpace << std::endl;
	std::cout << "\tNext: " << next << std::endl;
	std::cout << "\t# pages: " << numPages << std::endl;
	std::cout << "\t# files: " << numFiles << std::endl;
	std::cout << "\tFirst free block: " << firstFree << std::endl;
	std::cout << "\tData: " << data << std::endl;
	for (uint64_t page = 0; page < numPages; ++page) {
		for (uint64_t block=0; block < BLOCKS_PER_PAGE; ++block) {
			in.read(reinterpret_cast<char*>(&bid), sizeof(uint64_t));
			in.read(reinterpret_cast<char*>(&usedSpace), sizeof(uint64_t));
			in.read(reinterpret_cast<char*>(&next), sizeof(uint64_t));
			in.read(data, BLOCK_SIZE);
			std::cout << "Block:" << std::endl;
			std::cout << "\tBlock ID: " << bid << std::endl;
			std::cout << "\tUsed space: " << usedSpace << std::endl;
			std::cout << "\tNext: " << next << std::endl;
			std::cout << "\tData: " << data << std::endl;
		}
	}
	in.close();
	return 0;
}
