#include <sys/mman.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "Filesystem.h"
#include <string.h>

/*
	Constructor--
	Checks if the filesystem and metadata exist.  If not, it creates an initial page of data.
	If the files exist, load the metadata.
*/

Storage::Filesystem::Filesystem(std::string meta_, std::string data_): meta_fname(meta_), data_fname(data_) {
	// Initialize the filesystem
	bool create_initial = false;
	if (!file_exists(meta_) || !file_exists(data_)) {
		create_initial = true;
	}

	metadata.fd = open(meta_fname.c_str(), O_RDWR | O_CREAT, (mode_t)0644);
	if (metadata.fd == -1) {
		std::cerr << "Error opening metadata!" << std::endl;
		exit(1);
	}
	filesystem.fd = open(data_fname.c_str(), O_RDWR | O_CREAT, (mode_t)0644);
	if (filesystem.fd == -1) {
		std::cerr << "Error opening filesystem!" << std::endl;
		exit(1);
	}

	initFilesystem(create_initial);
}

/*
	Calculates the total size used by a chain of blocks.
*/

uint64_t Storage::Filesystem::calculateSize(Block b) {
	uint64_t size = 0;
	bool done = false;
	while (!done) {
		size += b.used_space;
		uint64_t next = b.next;
		if (next == 0) {
			done = true;
		} else {
			b = loadBlock(next);
		}
	}
	return size;
}

/*
	Load file metadata, size and first block location.
	If the file doesn't exist, create it.
*/

File Storage::Filesystem::load(std::string name) {
	if (metadata.files.count(name)) {
		uint64_t block = metadata.files[name];
		Block b = loadBlock(block);
		uint64_t size = calculateSize(b);
		File file(name, block, size);
		return file;
	} else {
		File file = createNewFile(name);
		return file;
	}
}

/*
	Write data to a file.
*/

void Storage::Filesystem::write(File *file, const char *data, uint64_t len) {
	uint64_t to_write = len;
	uint64_t pos = 0;
	Block block = loadBlock(file->block);
	file->size = len;
	
	while (to_write > 0) {
		if (to_write > BLOCK_SIZE) {
			memcpy(block.buffer, data + pos, BLOCK_SIZE);
			// Grab a new block
			if (block.next == 0) {
				block.next = getBlock();
			}
			block.used_space = BLOCK_SIZE;
			writeBlock(block);
			block = loadBlock(block.next);
			to_write -= BLOCK_SIZE;
			pos += BLOCK_SIZE;
		} else {
			memcpy(block.buffer, data + pos, to_write);
			// TODO: Need to add the leftover blocks to the free list...  Now any blocks leftover are essentially garbage
			if (block.next != 0) {
                // BUG: Just lost all other free blocks
				metadata.firstFree = block.next;
			}
			block.next = 0;
			block.used_space = to_write;
			writeBlock(block);
			to_write = 0;
			pos += to_write;
		}
	}	
}

/*
	Read (ALL) data from a file.
*/

char *Storage::Filesystem::read(File *file) {
	// Assume the file is just one block.  Realloc later if not the case
	char *buffer = (char*)malloc(file->size);
	uint64_t read_size = 0;
	Block block = loadBlock(file->block);
	bool done = false;
	while (!done) {
		memcpy(buffer + read_size, block.buffer, block.used_space);
		read_size += block.used_space;
		if (block.next != 0) {
			block = loadBlock(block.next);
		} else {
			done = true;
		}
	}
	return buffer;
}

/*
	Creates a file and syncs the metadata
*/

File Storage::Filesystem::createNewFile(std::string name) {
	File file(name, getBlock(), 0);
	metadata.files[name] = file.block;
	metadata.numFiles++;
	writeMetadata();
	return file;
}

/*
	Copy the contents of one block of data into a buffer along with block metadata.
*/

Block Storage::Filesystem::loadBlock(uint64_t blockID) {
	Block block;
	uint64_t id = blockID-1;
	memcpy(&block.id, filesystem.data + (id * sizeof(Block)), sizeof(uint64_t));
	memcpy(&block.used_space, filesystem.data + (id * sizeof(Block)) + sizeof(uint64_t), sizeof(uint64_t));
	memcpy(&block.next, filesystem.data + (id * sizeof(Block)) + 2*sizeof(uint64_t), sizeof(uint64_t));
	memcpy(block.buffer, filesystem.data + (id * sizeof(Block)) + 3*sizeof(uint64_t), BLOCK_SIZE);
	block.id = blockID;
	return block;
}

/*
	Replace a block with new data.
*/

void Storage::Filesystem::writeBlock(Block block) {
	uint64_t id = block.id-1;
	uint64_t position = id * sizeof(Block);
	while (position > filesystem.numPages * PAGESIZE) {
		growFilesystem();
	}
	memcpy(filesystem.data + id * sizeof(Block), &block.id, sizeof(uint64_t));
	memcpy(filesystem.data + id * sizeof(Block) + sizeof(uint64_t), &block.used_space, sizeof(uint64_t));
	memcpy(filesystem.data + id * sizeof(Block) + 2*sizeof(uint64_t), &block.next, sizeof(uint64_t));
	memcpy(filesystem.data + id * sizeof(Block) + 3*sizeof(uint64_t), block.buffer, BLOCK_SIZE);
}

/*
	Increase the size of the filesystem by an increment of one page.
*/

void Storage::Filesystem::growFilesystem() {
	std::cout << "Growing" << std::endl;
	posix_fallocate(filesystem.fd, PAGESIZE * filesystem.numPages, PAGESIZE);
	filesystem.data = (char*)t_mremap(filesystem.fd,
					filesystem.data,
					PAGESIZE * filesystem.numPages, 
					PAGESIZE * filesystem.numPages+1,
					MREMAP_MAYMOVE);
	filesystem.numPages++;
	uint64_t firstBlock = BLOCKS_PER_PAGE * (filesystem.numPages-1) + 1;
	chainPage(firstBlock);
}

/*
	Increase the size of the metadata by an increment of one page.
*/

void Storage::Filesystem::growMetadata() {
	posix_fallocate(metadata.fd, PAGESIZE * metadata.numPages, PAGESIZE);
	filesystem.data = (char*)t_mremap(metadata.fd,
					metadata.data,
					PAGESIZE * metadata.numPages, 
					PAGESIZE * metadata.numPages+1,
					MREMAP_MAYMOVE);
	metadata.numPages++;
}

/*
	If there is a block available, return it.
	Expand the filesystem if there are no blocks available.
*/

uint64_t Storage::Filesystem::getBlock() {
	uint64_t bid;
	Block b;
	// If there is a free block, use it.
	if (metadata.firstFree) {
		bid = metadata.firstFree;
		b = loadBlock(bid);
		metadata.firstFree = b.next;
	} else {
		// Grow the filesystem
		growFilesystem();
		bid = (BLOCKS_PER_PAGE * (filesystem.numPages-1)) + 1;
		b = loadBlock(metadata.firstFree);
		metadata.firstFree = b.next;
	}
	return bid;
}

/*
	Create the initial filesystem.
*/

void Storage::Filesystem::initFilesystem(bool initialFill) {
	// If the files were empty, fill them to one page
	if (initialFill) {
		posix_fallocate(metadata.fd, 0, PAGESIZE);
		posix_fallocate(filesystem.fd, 0, PAGESIZE);
	}

	// Map the metadata
	metadata.data = (char*)mmap64((caddr_t)0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, metadata.fd, 0);
	close(metadata.fd);
	if (!metadata.data) {
		std::cerr << "Error mapping metadata!" << std::endl;
		exit(1);
	}

	// Map the filesystem
	filesystem.data = (char*)mmap64((caddr_t)1, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, filesystem.fd, 0);
	close(filesystem.fd);
	if (!filesystem.data) {
		std::cerr << "Error mapping filesystem!" << std::endl;
		exit(1);
	}

	initMetadata();
	if (initialFill) {
		writeMetadata();
		chainPage(1);
	} else {
		readMetadata();
	}
}

/*
	Chain a page of blocks together.  TODO: What to do with the last block of the previous page?  Need to chain it to the first block of the next page...
*/

void Storage::Filesystem::chainPage(uint64_t startBlock) {
	Block b;
	b.id = startBlock;
	b.used_space = 0;
	b.next = startBlock + 1;
    std::fill( b.buffer , b.buffer + BLOCK_SIZE, 0 );
    /* for (int i=0; i<BLOCK_SIZE; ++i) {
		b.buffer[i] = '\0';
	} */
	uint64_t id;
	uint64_t begin = startBlock - 1;
	for (uint64_t i=0; i<BLOCKS_PER_PAGE; ++i) {
		id = b.id-1 + begin;
		memcpy(filesystem.data + id * sizeof(Block) + 0*sizeof(uint64_t), &b.id, sizeof(uint64_t));
		memcpy(filesystem.data + id * sizeof(Block) + 1*sizeof(uint64_t), &b.used_space, sizeof(uint64_t));
		memcpy(filesystem.data + id * sizeof(Block) + 2*sizeof(uint64_t), &b.next, sizeof(uint64_t));
		memcpy(filesystem.data + id * sizeof(Block) + 3*sizeof(uint64_t), b.buffer, BLOCK_SIZE);
		b.id++;
		if (i == BLOCKS_PER_PAGE - 1) {
			b.next = 0;
		} else {
			b.next++;
		}
	}
}

/*
	Set some initial values for the metadata.
*/

void Storage::Filesystem::initMetadata() {
	// Initial values
	metadata.numPages = 1;
	metadata.numFiles = 0;
	metadata.firstFree = 1;
	filesystem.numPages = 1;
}

/*
	Read the metadata into memory.
*/

void Storage::Filesystem::readMetadata() {
	// Ensure the header is intact
	for (uint64_t i = 0; i < sizeof(HEADER); ++i) {
		if (metadata.data[i] != HEADER[i]) {
			std::cerr << "Invalid header!" << std::endl;
			exit(1);
		}
	}
	memcpy(&metadata.numPages, metadata.data + sizeof(HEADER), sizeof(uint64_t));
	memcpy(&filesystem.numPages, metadata.data + sizeof(HEADER) + sizeof(uint64_t), sizeof(uint64_t));
	memcpy(&metadata.numFiles, metadata.data + sizeof(HEADER) + 2*sizeof(uint64_t), sizeof(uint64_t));
	memcpy(&metadata.firstFree, metadata.data + sizeof(HEADER) + 3*sizeof(uint64_t), sizeof(uint64_t));

	// If we have more than one page, remap the data
	if (metadata.numPages > 1) {
		metadata.data = (char*)t_mremap(metadata.fd,
						metadata.data,
						PAGESIZE,
						PAGESIZE * metadata.numPages,
						MREMAP_MAYMOVE);
	}

	if (filesystem.numPages > 1) {
		filesystem.data = (char*)t_mremap(filesystem.fd,
						filesystem.data, 
						PAGESIZE,
						PAGESIZE * filesystem.numPages,
						MREMAP_MAYMOVE);
	}

    // Loading the files?
	uint64_t pos = sizeof(HEADER) + 4*sizeof(uint64_t);
	char *buf = NULL;
	for (uint64_t i=0; i < metadata.numFiles; ++i) {
		// Read filename size
		uint64_t fnameSize;
		memcpy(&fnameSize, metadata.data + pos, sizeof(uint64_t));
		pos += sizeof(uint64_t);

		// Read file name
		buf = (char*)realloc(buf, fnameSize);
		memcpy(buf, metadata.data + pos, fnameSize);
		std::string key(buf, fnameSize);
		pos += fnameSize;

		// Read block ID
		uint64_t val = 0;
		memcpy(&val, metadata.data + pos, sizeof(uint64_t));
		pos += sizeof(uint64_t);

		// Insert into file map
		metadata.files[key] = val;
	}
	free(buf);
}

/*
	Write the metadata to disk.
*/

void Storage::Filesystem::writeMetadata() {
	memcpy(metadata.data, HEADER, sizeof(HEADER));
	memcpy(metadata.data + sizeof(HEADER), &metadata.numPages , sizeof(uint64_t));
	memcpy(metadata.data + sizeof(HEADER) + sizeof(uint64_t), &filesystem.numPages , sizeof(uint64_t));
	memcpy(metadata.data + sizeof(HEADER) + 2*sizeof(uint64_t), &metadata.numFiles , sizeof(uint64_t));
	memcpy(metadata.data + sizeof(HEADER) + 3*sizeof(uint64_t), &metadata.firstFree , sizeof(uint64_t));
	uint64_t pos = sizeof(HEADER) + 4*sizeof(uint64_t);
	for (auto it = metadata.files.begin(); it != metadata.files.end(); it++) {
		std::string key = it->first;
		uint64_t keySize = key.size();
		uint64_t val = it->second;

		// Write the size of the filename
		memcpy(metadata.data + pos, &keySize, sizeof(uint64_t)); 
		pos += sizeof(uint64_t);

		// Write the filename
		memcpy(metadata.data + pos, key.c_str(), keySize); 
		pos += keySize;
		
		// Write the block ID
		memcpy(metadata.data + pos, &val, sizeof(uint64_t)); 
		pos += sizeof(uint64_t);

		// Need to grow
		while (pos >= PAGESIZE * metadata.numPages) {
			growMetadata();
		} 
	}
}

/*
	Unmap the filesystem.
*/

void Storage::Filesystem::shutdown() {
	writeMetadata();
	munmap(metadata.data, metadata.numPages * PAGESIZE);
	munmap(filesystem.data, filesystem.numPages * PAGESIZE);
}
