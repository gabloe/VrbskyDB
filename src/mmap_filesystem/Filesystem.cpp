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
#include "HerpmapReader.h"
#include "HerpmapWriter.h"
//#include "HashmapReader.h"
//#include "HashmapWriter.h"


#define printJunk(MSG) {   \
    std::cout << MSG << std::endl;\
    std::cout << "\tNumPages: " << filesystem.numPages << std::endl;\
    std::cout << "\tnumFiles: " << metadata.numFiles << std::endl;\
    std::cout << "\tfirstFree: " << metadata.firstFree << std::endl;\
}

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

uint64_t Storage::Filesystem::getNumPages() {
	return filesystem.numPages;
}

uint64_t Storage::Filesystem::getNumFiles() {
	return metadata.numFiles;
}

/*
std::map<std::string, uint64_t> Storage::Filesystem::getFileMap() {
	return metadata.files;
}
*/

Storage::HerpHash<std::string, uint64_t> Storage::Filesystem::getFileMap() {
	return metadata.files;
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

void Storage::Filesystem::compact() {
    Filesystem *fs = new Filesystem("_compact.db");
    uint64_t oldNumFiles = metadata.files.size();
    uint64_t pos = 0;
    for (auto it = metadata.files.begin(); it != metadata.files.end(); ++it) {
	std::string key = it->first;

	// Don't copy over the metadata
	if (key.compare("_METADATA_") == 0) {
		continue;
	}

	File src = open_file(key);
	char *buffer = read(&src);
	File dest = fs->open_file(key);
	fs->write(&dest, buffer, src.size);
        std::cout << "Compacting: " << ceil(100 * (long double)pos / oldNumFiles) << "% done.\r";
	pos++;
    }
    std::cout << std::endl;
    uint64_t newNumPages = fs->getNumPages();
    uint64_t newNumFiles = fs->getNumFiles();
    //std::map<std::string, uint64_t> newFiles = fs->getFileMap();
    Storage::HerpHash<std::string,uint64_t> newFiles = fs->getFileMap();
    fs->shutdown();

    // Unmap the old filesystem
    munmap(filesystem.data, filesystem.numPages * PAGESIZE);

    metadata.files = newFiles;
    filesystem.numPages = newNumPages; 
    metadata.numFiles = newNumFiles; 

    // Close the old filesystem
    close(filesystem.fd);
    std::remove(data_fname.c_str());
    std::rename("_compact.db", data_fname.c_str());

    filesystem.fd = open(data_fname.c_str(), O_RDWR | O_CREAT, (mode_t)0644);
    if (!filesystem.fd) {
        std::cerr << "Could not reopen filesystem after compact!" << std::endl;
	exit(1);
    }

    initFilesystem(false); 
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

        // If we have more to write we need 
        // to go to the next block
        if( to_write > 0 ) {
            if ( block.next == 0) { // Need a new block 
                block.next = getBlock();
            }else { // already havea new block
                block.next = block.next;
            }
            writeBlock(block);
            block = loadBlock( block.next );
        }
    }

    // if the last block has more blocks connected we need
    // to put those on the free list.
    if( block.next != 0 ) {
        addToFreeList( block.next );
        block.next = 0;
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
    } else {
        // Append to end
        Block b = loadBlock(metadata.firstFree);
        while (b.next != 0) {
            b = loadBlock(b.next);
        }
        SET_FREE(b.next,block);
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
        Assert( "Did not remove?" , metadata.files.count(file->name) == 0 );
        addToFreeList(file->block);
        metadata.numFiles--;
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

    // How can this even happen?
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

    // Free list is empty.  Grow the filesystem.
    if (metadata.firstFree == 0) {
        growFilesystem();
    }

    // We know there is space available
    uint64_t bid = metadata.firstFree;
    Block b = loadBlock(bid);

    // update free list
    SET_FREE(metadata.firstFree,b.next);

    // update block
    b.next = 0;
    b.used_space = 0;
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
    uint64_t i;
    b.used_space = 0;
    std::fill( b.buffer , b.buffer + BLOCK_SIZE, 0 );
    for (i=0; i<BLOCKS_PER_PAGE - 1; ++i) {
        b.id = i + startBlock;
        b.next = b.id + 1;
        writeBlock(b);
    }
    // The last block in the chain should point to 0
    b.id = i + startBlock;
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
    filesystem.numPages = 1;
}

/*
   Read the metadata into memory.
   */

void Storage::Filesystem::readMetadata() {
    uint64_t pos = 0;
    uint64_t offset = 3 * sizeof(uint64_t); // Skip ID, next, and length
    filesystem.numPages = Read64( filesystem.data , offset );

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
    uint64_t metadata_size = calculateSize(b);

    metadata.file = File("__METADATA__", 1, metadata_size);

    char *buffer = read(&metadata.file);
    filesystem.numPages = Read64( buffer , pos );
    metadata.numFiles = Read64( buffer , pos );
    metadata.firstFree = Read64( buffer , pos );

    //printJunk("ReadMetaData");

    Assert( "position is wrong" , pos == 3 * sizeof(uint64_t) );
    HerpmapReader<uint64_t> reader(metadata.file, this);
    metadata.files = reader.read_buffer(buffer, pos, metadata_size);
    free(buffer);
}

/*
   Write the metadata to disk.
   */

void Storage::Filesystem::writeMetadata() {

    // variables
    uint64_t size,pos,files_size;
    HerpmapWriter<uint64_t> writer(metadata.file, this);
    char *files,*buf;

    // Get files
    files = writer.write_buffer(metadata.files, &files_size);

    // Allocate buffer
    size = (3 * sizeof(uint64_t)) + files_size;
    buf = new char[size];

    // Write numPages first
    pos = 0;
    Write64(buf , pos , filesystem.numPages);
    Write64(buf , pos , metadata.numFiles);
    Write64(buf , pos , metadata.firstFree);
    WriteRaw(buf, pos , files , files_size );

    uint64_t test = filesystem.numPages + metadata.firstFree + metadata.numFiles;
    //printJunk("writeMetadata");
    write(&metadata.file, buf, size);
    test -= (filesystem.numPages + metadata.firstFree + metadata.numFiles);

    if(test) {
        pos = 0;
        Write64(buf , pos , filesystem.numPages);
        Write64(buf , pos , metadata.numFiles);
        Write64(buf , pos , metadata.firstFree);
        //printJunk("writeMetadata2");
        write(&metadata.file, buf, size);
    }
    free(files);
    delete[] buf;
}

/*
   Unmap the filesystem.
*/

void Storage::Filesystem::shutdown() {
    writeMetadata();
    close(filesystem.fd);
    munmap(filesystem.data, filesystem.numPages * PAGESIZE);
}
