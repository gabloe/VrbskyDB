#include <sys/mman.h>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "../assert/Assert.h"
#include "Filesystem.h"
#include "HashmapReader.h"
#include "HashmapWriter.h"

#define SET_FREE(SPOT,NEW) {    \
    SPOT = NEW;                 \
}

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

File Storage::Filesystem::open_file(std::string name) {
    if (metadata.files.count(name)) {
        uint64_t block = metadata.files[name];
        Block b = loadBlock(block);
        uint64_t size = calculateSize(b);
        File file(name, block, size);
        return file;
    } else {
        return createNewFile(name);
    }
}

/*
   Write data to a file.
   */

#define ALT 1

#if ALT 
void Storage::Filesystem::write(File *file, const char *data, uint64_t len) {
    uint64_t to_write = len;
    uint64_t pos = 0;
    Block block = loadBlock(file->block);

    while (to_write > 0) {
        // How much are we going to shove in this block
        uint64_t t_w = std::min( to_write , BLOCK_SIZE );

        // Shove it
        memcpy(block.buffer, data + pos, t_w );
        block.used_space = t_w;

        // Update as neccessary
        to_write -= t_w;
        pos += t_w;

        if( to_write > 0 ) {
            uint64_t next;
            if ( block.next == 0) { 
                next = getBlock();
            }else {
                next = block.next;
            }
            block.next = next;
            writeBlock(block);
            block = loadBlock( next );
        }
    }
    // Write last block
    writeBlock(block);
    file->size = len;
}
#else
void Storage::Filesystem::write(File *file, const char *data, uint64_t len) {
    uint64_t to_write = len;
    uint64_t pos = 0;
    Block block = loadBlock(file->block);

    while (to_write > 0) {
        if (to_write > BLOCK_SIZE) {
            memcpy(block.buffer, data + pos, BLOCK_SIZE);
            // Grab a new block
            uint64_t next = block.next;
            if (next == 0) {
                next = getBlock();
            }
            block.used_space = BLOCK_SIZE;
            block.next = next;
            writeBlock(block);
            block = loadBlock(block.next);
            to_write -= BLOCK_SIZE;
            pos += BLOCK_SIZE;
        } else {
            memcpy(block.buffer, data + pos, to_write);
            block.next = 0;
            block.used_space = to_write;
            writeBlock(block);
            to_write = 0;
            pos += to_write;
        }
    }	
    file->size = len;
}
#endif

void Storage::Filesystem::addToFreeList(uint64_t block) {
    if (metadata.firstFree == 0) {
        SET_FREE(metadata.firstFree,block);
        //metadata.firstFree = block;
    } else {
        // Go to last block and append
        Block b = loadBlock(metadata.firstFree);
        while (b.next != 0) {
            b = loadBlock(b.next);
        }
        SET_FREE(b.next,block);
        //b.next = block;
        writeBlock(b);
    }
}

std::vector<std::string> Storage::Filesystem::getFilenames() {
    std::vector<std::string> res;
    for (auto it = metadata.files.begin(); it != metadata.files.end(); ++it) {
        if( it->first != "__METADATA__" ) {
            res.push_back(it->first);
        }
    }
    return res;
}

bool Storage::Filesystem::deleteFile(File *file) {
    if (metadata.files.count(file->name)) {
	metadata.files.erase(file->name);
        addToFreeList(file->block);
	return true;
    } else {
	return false;
    }
}

/*
   Read (ALL) data from a file.
   */

char *Storage::Filesystem::read(File *file) {
    if (file->size == 0) {
        return NULL;
    }
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
    return file;
}

/*
   Copy the contents of one block of data into a buffer along with block metadata.
   */

Block Storage::Filesystem::loadBlock(uint64_t blockID) {
    Block block;
    uint64_t id = blockID - 1;
    uint64_t offset = id * BLOCK_SIZE_ACTUAL;
    uint64_t pos = offset;

    memcpy(&block.id, filesystem.data + pos, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(&block.used_space, filesystem.data + pos, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(&block.next, filesystem.data + pos ,sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(block.buffer, filesystem.data + pos, BLOCK_SIZE);
    pos += BLOCK_SIZE;

    assert(pos-offset == BLOCK_SIZE_ACTUAL);

    return block;
}

/*
   Replace a block with new data.
   */

void Storage::Filesystem::writeBlock(Block block) {
    uint64_t id = block.id - 1;
    while (id >= filesystem.numPages * BLOCKS_PER_PAGE) {
        growFilesystem();
    }
    uint64_t pos = id * BLOCK_SIZE_ACTUAL;
    memcpy(filesystem.data + pos, &block.id, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(filesystem.data + pos, &block.used_space, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(filesystem.data + pos, &block.next, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(filesystem.data + pos, block.buffer, BLOCK_SIZE);
    pos += BLOCK_SIZE;

    msync(filesystem.data + id * BLOCK_SIZE_ACTUAL, BLOCK_SIZE_ACTUAL, MS_SYNC);
}

/*
   Increase the size of the filesystem by an increment of one page.
   */

void Storage::Filesystem::growFilesystem() {
    posix_fallocate(filesystem.fd, PAGESIZE * filesystem.numPages, PAGESIZE * (filesystem.numPages+1));
    filesystem.data = (char*)t_mremap(filesystem.fd,
            filesystem.data,
            PAGESIZE * filesystem.numPages, 
            PAGESIZE * (filesystem.numPages+1),
            MREMAP_MAYMOVE);

    if( !filesystem.data ) {
        std::cerr << "Error when growing filesystem" << std::endl;
        std::exit( -1 );
    }

    filesystem.numPages++;
    uint64_t firstBlock = (BLOCKS_PER_PAGE * (filesystem.numPages-1)) + 1;
    chainPage(firstBlock);

    // Add page to free list
    addToFreeList(firstBlock);
    //writeMetadata();
}

/*
   If there is a block available, return it.
   Expand the filesystem if there are no blocks available.
   */

uint64_t Storage::Filesystem::getBlock() {
    uint64_t bid;
    Block b;
    // Free list is empty.  Grow the filesystem.
    if (metadata.firstFree == 0) {
        growFilesystem();
    }

    // We know there is space available
    bid = metadata.firstFree;
    b = loadBlock(bid);
    SET_FREE(metadata.firstFree,b.next);
    //metadata.firstFree = b.next;
    b.next = 0;
    writeBlock(b);
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
    filesystem.data = (char*)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, filesystem.fd, 0);
    if (!filesystem.data) {
        std::cerr << "Error mapping filesystem!" << std::endl;
        exit(1);
    }

    initMetadata();
    if (initialFill) {
        chainPage(1);
        metadata.file = open_file("__METADATA__");
        writeMetadata();
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
    for (uint64_t i=0; i<BLOCKS_PER_PAGE; ++i) {
        b.id = i + startBlock;
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
    metadata.numFiles = 0;
    SET_FREE(metadata.firstFree,1);
    //metadata.firstFree = 1;
    filesystem.numPages = 1;
}

/*
   Read the metadata into memory.
   */

void Storage::Filesystem::readMetadata() {
    uint64_t pos = 0;
    uint64_t offset = 3 * sizeof(uint64_t); // Skip ID, next, and length
    memcpy(&filesystem.numPages,    filesystem.data + offset , sizeof(uint64_t) );

    if (filesystem.numPages > 1) {
        filesystem.data = (char*)t_mremap(filesystem.fd,
                filesystem.data, 
                PAGESIZE,
                PAGESIZE * filesystem.numPages,
                MREMAP_MAYMOVE);
        if(!filesystem.data) {
            std::cerr << "Error remapping" << std::endl;
            std::exit(-1);
        }
    }


    Block b = loadBlock(1);
    uint64_t tmpData = 0;
    uint64_t metadata_size = calculateSize(b);
    metadata.file = File("__METADATA__", 1, metadata_size);

    char *buffer = read(&metadata.file);
    // Skip numPages
    pos = 1 * sizeof(uint64_t);
    memcpy(&metadata.numFiles, buffer + pos, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    memcpy(&tmpData, buffer + pos, sizeof(uint64_t));
    SET_FREE(metadata.firstFree,tmpData);
    pos += sizeof(uint64_t);

    HashmapReader<uint64_t> reader(metadata.file, this);
    metadata.files = reader.read_buffer(buffer, pos, metadata_size);
    free(buffer);
    if( 0 ) {
        std::cout << "Reading Metadata" << std::endl;
        std::cout << "\tnum pages:" << filesystem.numPages << std::endl;
        std::cout << "\tnum files:" << metadata.numFiles << std::endl;
        std::cout << "\tFirst free: " << metadata.firstFree << std::endl;
        std::cout << "\tRead size is " << metadata_size << std::endl;
    }
}

/*
   Write the metadata to disk.
   */
#define printJunk() {   \
    std::cout << "Writing metadata" << std::endl;\
    std::cout << "\tNumPages: " << filesystem.numPages << std::endl;\
    std::cout << "\tnumFiles: " << metadata.numFiles << std::endl;\
    std::cout << "\tfirstFree: " << metadata.firstFree << std::endl;\
}

void Storage::Filesystem::writeMetadata() {
    HashmapWriter<uint64_t> writer(metadata.file, this);	
    uint64_t size = 3 * sizeof(uint64_t);
    uint64_t pos = 0;
    uint64_t files_size = 0;

    //printJunk();
    
    uint64_t test = filesystem.numPages + metadata.firstFree + metadata.numFiles;


    char *files = writer.write_buffer(metadata.files, &files_size);
    size += files_size;
    char *buf = (char*)malloc(size);

    // Write numPages first
    memcpy(buf+pos, &filesystem.numPages, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    // Write numFiles second
    memcpy(buf+pos, &metadata.numFiles, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    // Write firstFree third
    memcpy(buf+pos, &metadata.firstFree, sizeof(uint64_t));
    pos += sizeof(uint64_t);

    // Write data fourth
    memcpy(buf+pos, files, files_size);
    pos += files_size;

    write(&metadata.file, buf, size);

    test -= (filesystem.numPages + metadata.firstFree + metadata.numFiles);
    if(test) {
        memcpy(buf + 0 * sizeof(uint64_t), &filesystem.numPages, sizeof(uint64_t));
        memcpy(buf + 1 * sizeof(uint64_t), &metadata.numFiles, sizeof(uint64_t));
        memcpy(buf + 2 * sizeof(uint64_t), &metadata.firstFree, sizeof(uint64_t));
        write(&metadata.file, buf, size);
    }
    free(files);
    free(buf);
}

/*
   Unmap the filesystem.
   */

void Storage::Filesystem::shutdown() {
    writeMetadata();
    close(filesystem.fd);
    munmap(filesystem.data, filesystem.numPages * PAGESIZE);
}
