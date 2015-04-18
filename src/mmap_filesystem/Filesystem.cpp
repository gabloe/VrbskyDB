#include <sys/mman.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "Filesystem.h"
#include "HashmapReader.h"
#include "HashmapWriter.h"
#include <string.h>

/*
	Constructor--
	Checks if the filesystem and metadata exist.  If not, it creates an initial page of data.
	If the files exist, load the metadata.
*/

Storage::Filesystem::Filesystem(std::string data_): data_fname(data_) {
	// Initialize the filesystem
	bool create_initial = false;
	if (!file_exists(data_)) {
		create_initial = true;
	}

	filesystem.fd = open(data_fname.c_str(), O_RDWR | O_CREAT, (mode_t)0777);
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
			uint64_t next = getBlock();
			block.used_space = BLOCK_SIZE;
			block.next = next;
			writeBlock(block);
			block = loadBlock(block.next);
			to_write -= BLOCK_SIZE;
			pos += BLOCK_SIZE;
		} else {
			memcpy(block.buffer, data + pos, to_write);
			// If the chain had more
			if (block.next != 0) {
				//addToFreeList(block.next);
			}
			block.next = 0;
			block.used_space = to_write;
			writeBlock(block);
			to_write = 0;
			pos += to_write;
		}
	}	
}

void Storage::Filesystem::addToFreeList(uint64_t block) {
	std::cout << "Adding " << block << " to free list" << std::endl;
	if (metadata.firstFree == 0) {
		metadata.firstFree = block;
	} else {
		Block b = loadBlock(metadata.firstFree);
		while (b.next != 0) {
			b = loadBlock(b.next);
		}
		b.next = block;
		writeBlock(b);
	}
	writeMetadata();
}

/*
	Read (ALL) data from a file.
*/

char *Storage::Filesystem::read(File *file) {
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
	while (id >= filesystem.numPages * BLOCKS_PER_PAGE) {
		growFilesystem();
	}
	memcpy(filesystem.data + id * sizeof(Block), &block.id, sizeof(uint64_t));
	memcpy(filesystem.data + (id * sizeof(Block)) + sizeof(uint64_t), &block.used_space, sizeof(uint64_t));
	memcpy(filesystem.data + (id * sizeof(Block)) + 2*sizeof(uint64_t), &block.next, sizeof(uint64_t));
	memcpy(filesystem.data + (id * sizeof(Block)) + 3*sizeof(uint64_t), block.buffer, BLOCK_SIZE);
}

/*
	Increase the size of the filesystem by an increment of one page.
*/

void Storage::Filesystem::growFilesystem() {
	filesystem.data = (char*)t_mremap(filesystem.fd,
					filesystem.data,
					PAGESIZE * filesystem.numPages, 
					PAGESIZE * (filesystem.numPages+1),
					MREMAP_MAYMOVE);
	filesystem.numPages++;
	uint64_t firstBlock = (BLOCKS_PER_PAGE * (filesystem.numPages-1)) + 1;
	chainPage(firstBlock);
	// Add page to free list
	addToFreeList(firstBlock);
}

/*
	If there is a block available, return it.
	Expand the filesystem if there are no blocks available.
*/

uint64_t Storage::Filesystem::getBlock() {
	uint64_t bid;
	Block b;
	if (metadata.firstFree == 0) {
		growFilesystem();
	}
	bid = metadata.firstFree;
	b = loadBlock(bid);
	metadata.firstFree = b.next;
	return bid;
}

/*
	Create the initial filesystem.
*/

void Storage::Filesystem::initFilesystem(bool initialFill) {
	// If the files were empty, fill them to one page
	if (initialFill) {
		posix_fallocate(filesystem.fd, 0, PAGESIZE);
	}

	// Map the filesystem
	filesystem.data = (char*)mmap((caddr_t)0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, filesystem.fd, 0);
	if (!filesystem.data) {
		std::cerr << "Error mapping filesystem!" << std::endl;
		exit(1);
	}

	initMetadata();
	if (initialFill) {
		writeMetadata();
		chainPage(2);
	} else {
		readMetadata();
	}
}

/*
	Chain a page of blocks together.  TODO: What to do with the last block of the previous page?  Need to chain it to the first block of the next page...
*/

void Storage::Filesystem::chainPage(uint64_t startBlock) {
	Block b;
	b.used_space = 0;
    	std::fill( b.buffer , b.buffer + BLOCK_SIZE, 0 );
	for (uint64_t i=startBlock; i<startBlock+BLOCKS_PER_PAGE; ++i) {
		if (i > filesystem.numPages * BLOCKS_PER_PAGE) {
			break;
		}
		b.id = i;
		b.next = b.id+1;
		writeBlock(b);
	}
	// The last block in the chain should point to 0
	b = loadBlock(b.id-1);
	b.next = 0;
	writeBlock(b);
}

/*
	Set some initial values for the metadata.
*/

void Storage::Filesystem::initMetadata() {
	// Initial values
	metadata.numFiles = 1;
	metadata.firstFree = 2;
	filesystem.numPages = 1;
	Block meta_block = loadBlock(1);
	uint64_t meta_size = calculateSize(meta_block);
	metadata.file = File("_METADATA_", 1, meta_size);
	metadata.files["_METADATA_"] = 1;
}

/*
	Read the metadata into memory.
*/

void Storage::Filesystem::readMetadata() {
	HashmapReader<uint64_t> reader(metadata.file, *this);
	char *data = read(&metadata.file);
	uint64_t pos = 0;
	memcpy(&metadata.numFiles, data + pos, sizeof(uint64_t));
	pos += sizeof(uint64_t);

	memcpy(&metadata.firstFree, data + pos, sizeof(uint64_t));
	pos += sizeof(uint64_t);

	if (filesystem.numPages > 1) {
		filesystem.data = (char*)t_mremap(filesystem.fd,
						filesystem.data, 
						PAGESIZE,
						PAGESIZE * filesystem.numPages,
						MREMAP_MAYMOVE);
	}
	metadata.files = reader.read(pos);	
}

/*
	Write the metadata to disk.
*/

void Storage::Filesystem::writeMetadata() {
	HashmapWriter<uint64_t> writer(metadata.file, *this);	
	uint64_t size = 2 * sizeof(uint64_t);
	uint64_t pos = 0;
	char *buffer = (char*)malloc(size);
	memcpy(buffer+pos, &metadata.numFiles, sizeof(uint64_t));
	pos += sizeof(uint64_t);

	memcpy(buffer+pos, &metadata.firstFree, sizeof(uint64_t));
	pos += sizeof(uint64_t);

	char *files;
	uint64_t files_size = writer.write_buffer(metadata.files, files);
	size += files_size;
	buffer = (char*)realloc(buffer, size);
	memcpy(buffer+pos, files, files_size);	
	write(&metadata.file,buffer,size);
	free(files);
	free(buffer);
}

/*
	Unmap the filesystem.
*/

void Storage::Filesystem::shutdown() {
	writeMetadata();
	munmap(filesystem.data, filesystem.numPages * PAGESIZE);
	close(filesystem.fd);
}
