// std::copy
#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>

// std::min
#include <cmath>

#include "FileSystem.h"
#include "File.h"

bool fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}

// implementation details


// Given a block and a start and end we remove all the bytes inbetween
void compact( Block &b, uint64_t start , uint64_t end ) {
    if( start >= end || start >= b.length ) return;

    // Remove trailing data
    if( end >= b.length ) {
        b.length = start;
        return;
    }

    uint64_t len = end - start;
    for( int i = 0 ; i < length; ++i ) {
        b.data[start + i] = b.data[end + i];
    }
    b.length -= len;
}


// namespace implementation

namespace os {

    // Private functions

    /*
     *  Locking Functions
     */

    // Lock the file for writing
    void FileSystem::lock( LockType type ){
        switch( type ) {
            case READ:
                break;
            case WRITE:
                break;
        }
    }

    void FileSystem::unlock( LockType type ) {
        switch( type ) {
            case READ:
                break;
            case WRITE:
                break;
        }
    }


    /*
     *  Low level filesystem functions
     */

    bool FileSystem::rename( File &toRename , const std::string newName ) {
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
            File &f = *file;
            if( f.getFilename() == toRename.getFilename() ) {
                f.name = newName;
                toRename.name = newName;
                return true;
            }
        }
        return false;
    }
    

    void FileSystem::split( Block &b , uint64_t offset ) {
        if( offset == b.length ) return;

        Block newBlock = allocate( b.length - offset , b.data + offset );
        Block next = lazyLoad( b.next );

        newBlock.prev = b.block;
        newBlock.next = next.block;

        b.next = newBlock.block;
        next.prev = newBlock.block;

        b.length = offset;

        flush( newBlock );
        flush( b );
        flush( next );

    }

    //  grow    -   Grow the filesystem enough to support writing the number of bytes given
    //
    //  bytes   -   How much space we need
    //
    //  @return -   The first block of the new space
    //
    Block FileSystem::grow( uint64_t bytes , const char *buffer ) {

        uint64_t blocksToWrite, blockId;

        blocksToWrite = (bytes + BlockSize) / BlockSize;
        bytes = TotalBlockSize * blocksToWrite;

        // Grow file system
        lock( WRITE );
        {
            stream.seekp( -bytes + 1 , std::ios_base::end );
            stream.put( 0 );
            blockId = numBlocks;
            numBlocks += blocksToWrite;
            stream.seekp( LengthOffset , std::ios_base::beg );
            stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof(numBlocks) );
        }unlock( WRITE );

        // Data
        Block b;
        b.status = FULL;
        b.block = blockId;
        b.length = std::min( bytes , BlockSize );
        b.prev = blockId + blocksToWrite - 1;
        b.next = blockId + blocksToWrite - 1;
        for( int i = 0 ; i < b.length ; ++i ) {
            b.data[i] = buffer[i];
        }

        uint64_t previous = b.prev;
        // Begin our writing
        std::fstream secondary( fileSystemLocation );

        secondary.seekp( HeaderSize + TotalBlockSize * blockId , std::ios_base::beg );
        for( int i = 0 ; i < blocksToWrite - 1; ++i) {
            secondary.write( reinterpret_cast<char*>( previous ) , sizeof(uint64_t) ); // Prev 
            secondary.write( reinterpret_cast<char*>( blockId + 1 ) , sizeof(uint64_t) ); // Next 
            secondary.write( reinterpret_cast<char*>( BlockSize ) , sizeof(uint64_t) ); // Length
            secondary.write( reinterpret_cast<const char*>( buffer ) , BlockSize ); // Data 
            buffer += BlockSize;
            previous = blockId;
            ++blockId;
        }

        secondary.write( reinterpret_cast<char*>( previous ) , sizeof(uint64_t) ); // Prev 
        secondary.write( reinterpret_cast<char*>( 0 ) , sizeof(uint64_t) ); // Next 
        secondary.write( reinterpret_cast<char*>( bytes % BlockSize ) , sizeof(uint64_t) ); // Length
        secondary.write( reinterpret_cast<const char*>( buffer ) , bytes % BlockSize ); // Data 

        return b;
    }


    //  load   -   Given a block id we load it from disk if not already loaded
    // 
    //  block       -   Block id we want to load
    //
    //  @return     -   Loaded block
    //
    Block FileSystem::load( uint64_t block ) {
        Block b;
        b.block = block;
        b.status = FULL;
        lock( READ );
        {
            stream.seekg( HeaderSize + block * TotalBlockSize , std::ios_base::beg );
            stream.read( reinterpret_cast<char*>( &b.prev ) , sizeof(b.prev) );
            stream.read( reinterpret_cast<char*>( &b.next ) , sizeof(b.next) );
            stream.read( reinterpret_cast<char*>( &b.length ) , sizeof(b.length) );
            stream.read( reinterpret_cast<char*>(b.data) , BlockSize );
        }
        unlock( READ );
        return b;
    }

    //  lazyLoad   - Given a block id only load meta-data
    //  
    //  block           - The block we wish to load
    //
    //  @return         - The loaded block
    //
    // TODO: Caching
    //
    Block FileSystem::lazyLoad( uint64_t block ) {
        Block b;
        b.block = block;
        b.status = LAZY;
        lock( READ );
        {
            stream.seekg( HeaderSize + block * TotalBlockSize , std::ios_base::beg );
            stream.read( reinterpret_cast<char*>( &b.prev ) , sizeof(b.prev) );
            stream.read( reinterpret_cast<char*>( &b.next ) , sizeof(b.next) );
            stream.read( reinterpret_cast<char*>( &b.length ) , sizeof(b.length) );
        }
        unlock( READ );
        return b;
    }

    //  flush   -   Given a block we flush it to disk
    //
    //  b       -   The block, with its data, we want to save to disk
    //  
    //
    void FileSystem::flush( Block &b ) {

        lock( WRITE ); 
        {
            stream.seekp( HeaderSize + b.block * TotalBlockSize , std::ios_base::beg );
            stream.write( reinterpret_cast<char*>(&b.prev) , sizeof(b.prev) );
            stream.write( reinterpret_cast<char*>(&b.next) , sizeof(b.next) );
            stream.write( reinterpret_cast<char*>(&b.length) , sizeof(b.length) );
            if( b.status == FULL ) {
                stream.write( reinterpret_cast<char*>(b.data) , BlockSize );
            }
            stream.flush();
        }
        unlock( WRITE );
    }

    //  reuse   -   Given some amount of bytes and data we try to load blocks from free list
    //
    //      length and buffer is modified as we use data.  If not enough free blocks exist
    //      then length is the number of bytes that we could not store and buffer points 
    //      to the first element which we could not store.
    //
    //  length  -   Number of bytes of data
    //  buffer  -   data
    //
    //  @return -   The first block in the freelist
    Block FileSystem::reuse( uint64_t &length , const char* &buffer ) {
        Block head;

        if( numFreeBlocks > 0 ) {
            uint64_t prevBlock = 0,currBlock = freeList;
            Block current;

            head = lazyLoad( currBlock );

            do {
                // Update free list
                freeList = current.next;
                --numFreeBlocks;

                // Currently we treat the freeList as a special file, so just use it as such
                current = lazyLoad( currBlock );
                current.status = FULL;
                current.length = std::min( length , BlockSize );
                uint64_t fillOffset = 0;
                if( buffer != 0 ) {
                    std::copy( buffer , buffer + current.length , current.data );
                    fillOffset = BlockSize - current.length;
                }
                std::fill( current.data + fillOffset , current.data + BlockSize , 0 );

                // Update input
                buffer += current.length;
                length -= current.length;

                // Write to disk
                flush( current );

                // Go to next
                prevBlock = currBlock;
                currBlock = freeList;

            } while ( numFreeBlocks > 0 && length > 0 );

            current.next = 0;
            head.prev = prevBlock;
            flush( current );
            flush( head );
        }

        return head;
    }

    //  allocate    -   Allocate enough bytes to support writing the data given
    //
    //  length      -   The amount of data in bytes
    //  buffer      -   The data
    //
    //  @return     -   The first block which holds our data
    //
    Block FileSystem::allocate( uint64_t length , const char* buffer ) {
        Block head;
        if( length == 0 ) {
            return head;
        }

        if( numFreeBlocks > 0 ){
            Block head = reuse( length , buffer );
            if( length > 0 ) {
                Block grown = grow( length , buffer );

                // The very end
                uint64_t tmp = grown.prev;

                // Join in middle
                Block b_tmp = lazyLoad( head.prev );
                b_tmp.next = grown.block;
                grown.prev = head.prev;

                // Point to actual end
                head.prev = tmp;

            }
        }else {
            head = grow( length , buffer );
        }
        return head;
    }


    //  locate    -   Given a block id and an offset find the block which
    //                  has the data corresponding to that offset.
    //
    //  start       -   The starting block
    //  offset      -   How many bytes from the starting block
    //
    //  @return     -   The block which holds the data pointed at by the offset
    //
    Block FileSystem::locate( uint64_t start , uint64_t &offset ) {
        Block b = lazyLoad( start );
        while( offset >= b.length ) {
            offset -= b.length;
            if( b.next == 0 ) {
                b = Block();
                break;
            }
            b = lazyLoad( b.next );
        }
        return b;
    }

    //  read    -   Read some given number of bytes starting at the offset and 
    //              store it in the buffer provided
    //
    //  start   -   Starting block to measure offset from
    //  offset  -   Offset from starting block in bytes
    //  length  -   How many bytes to try to read
    //  buffer  -   Where to store the data
    //
    //  @return -   Number of bytes read
    uint64_t FileSystem::read( uint64_t start , uint64_t offset , uint64_t length , char* buffer ) {
        if( buffer == 0 ) return 0;
        Block b = locate( start , offset );
        if( b.block == 0 ) return 0;

        // Read data
        b = load( b.block );
        char *to = buffer;
        do {
            uint64_t len = std::min( b.length - offset , length );
            std::copy( b.data + offset , b.data + len , buffer );

            offset = 0;
            buffer += len;
            length -= len;

            if( b.next == 0 ) { // Last block, TODO: Fatal, throw exception?
                break;
            }
            b = load( b.next );
        }while( length > 0 );

        // How much we read
        return to - buffer;
    }

    //  write   -   Given some data write it at an offset from a starting block
    //
    //  start   -   The starting block
    //  offset  -   The nunber of bytes to our place we wish to write to
    //  length  -   How much data we want to write
    //  buffer  -   The data we want to write
    //
    //      If the buffer is NULL we write the NULL character.
    //
    //  @return -   How much data we wrote
    uint64_t FileSystem::write( uint64_t start , uint64_t offset , uint64_t length , const char* buffer ) {
        Block b = locate( start , offset );
        if( b.block == 0 ) return 0;

        const char *to = buffer;
        do {

            uint64_t len = std::min( BlockSize , length );
            std::copy( buffer , buffer + len , b.data + offset );
            b.status = FULL;
            offset = 0;

            // Update and flush to disk
            b.length = len;
            flush( b );

            // Next
            buffer += len;
            length -= len;

            if( b.next == 0 ) { // Last block
                Block next = allocate( length , buffer );
                next.prev = b.block;
                b.next = next.block;
                flush( next );
                flush( b );
                break;
            }else {
                b = lazyLoad( start );
            }

        }while( length > 0 );
        // How much we wrote
        return to - buffer;
    }

    //  insert  -   We insert data at a given offset in the file.  We do not write
    //              over any data.
    //
    //  start   - Starting block to begin search from
    //  offset  -   How many bytes to skip over
    //  length  -   The amount of data to insert
    //  buffer  -   The data to insert
    //
    //      If the buffer is NULL we write the null character
    //
    //  @return -   The number of bytes written
    uint64_t FileSystem::insert( uint64_t start , uint64_t offset , uint64_t length , const char* buffer ) {
        Block b = locate( start , offset );
        if( b.block == 0 ) return 0;

        // We might have to split
        split( b , offset % BlockSize );

        // Now append to here
        Block next = allocate( length , buffer );
        Block end = lazyLoad( next.prev );
        end.next = b.next;
        next.prev = b.block;
        b.next = next.block;

        flush( next );
        flush( end );
        flush( b );

        return length;
    }

    //
    //  releaseBlock    -   Adds a block to the freelist for later use
    //
    //  block           -   The block we are freeing
    //
    void releaseBlock( uint64_t block ) {
    }

    //
    //  remove  -   Remove bytes from a file
    //
    //  file    -   The file we want to remove bytes from
    //  length  -   How many bytes to remove
    //
    //  @return -   How many bytes actually removed
    //
    uint64_t FileSystem::remove( File &file ,uint64_t length ) {

        length = std::min( length , file.numBytes - file.byteOffset );
        if( length == 0 ) return 0;

        // Load blocks
        Block first = load( file.currentBlock );

        uint64_t removeFromFirst = std::min( length , first.length - file.blockOffset );
        uint64_t removeFromRemaining = length - removeFromFirst;

        Block last = locate( first.next , removeFromRemaining );
        last = load( last.block );

        // Compact blocks
        compact( first , file.blockOffset , removeFromFirst );
        compact( last , 0 , removeFromRemaining );

        if( first.next != last.block ) {
            releaseBlock( first.next );
        }

        first.next = last.block;
        last.prev = first.block;

        flush( first );
        flush( last );

    }


    // Public Methods/Functions

    void FileSystem::shutdown() {
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ) {
            (*file).close();
            openFiles.erase(file);
        }
    }

    std::list<File*> FileSystem::getFiles() {
        std::list<File*> ret;
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            ret.push_back( &(*file));
        }
        return ret;
    }

    std::list<File*> FileSystem::getOpenFiles() {
        std::list<File*> ret;
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
            ret.push_back( &(*file));
        }
        return ret;
    }

    std::list<std::string> FileSystem::getFilenames() {
        std::list<std::string> names;
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            names.push_back( (*file).getFilename() );
        } 
        return names;
    }

    File FileSystem::open( const std::string name) {
        File f;
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            if( (*file).getFilename() == name ) {
                return *file;
            }
        } 
        return f;
    }

    //
    bool FileSystem::unlink( const std::string filename ) {
        File file = open(filename);
        return file.unlink();
    }

    bool FileSystem::exists( const std::string filename ) {
        File file = open(filename);
        return file.getStatus() == OPEN;
    }


    FileSystem::FileSystem( const std::string filename) {

        fileSystemLocation = filename;
        //  Open the file
        stream.open( filename );

        if( !fileExists( filename ) ) {
            totalBytes = 0;
            freeList = 0;
            numBlocks = 0;
            numFreeBlocks = 0;
            numFiles = 0;

            stream.write( HeaderSignature , sizeof( SignatureSize ) );
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ) );
            stream.write( reinterpret_cast<char*>(&freeList) , sizeof( freeList ) );
            stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof( numBlocks) );
            stream.write( reinterpret_cast<char*>(&numFreeBlocks) , sizeof( numFreeBlocks ) );
            stream.write( reinterpret_cast<char*>(&numFiles) , sizeof( numFiles ) );

        }else {
            //  Read the header
            char buff[SignatureSize];
            stream.read( buff , HeaderSize );
            if( std::strncmp( buff , HeaderSignature , SignatureSize ) != 0 ) {
                std::cout << "Invalid header signature found" << std::endl;
                std::exit( -1 );
            }
            stream.read( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ) );
            stream.read( reinterpret_cast<char*>(&freeList) , sizeof( freeList ) );
            stream.read( reinterpret_cast<char*>(&numBlocks) , sizeof( numBlocks) );
            stream.read( reinterpret_cast<char*>(&numFreeBlocks) , sizeof( numFreeBlocks ) );
            stream.read( reinterpret_cast<char*>(&numFiles) , sizeof( numFiles ) );
            //  Read the filenames

        }

    }

    void FileSystem::closing( File *fp ) {
        for( auto file = openFiles.begin(); file != openFiles.end(); ++file ) {
            if( (*file).getFilename() == (*fp).getFilename() ) {
                openFiles.erase(file);
                return;
            }
        }
    }

    FileSystem::~FileSystem() {
        shutdown();
    }

};
