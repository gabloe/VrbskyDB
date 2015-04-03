// std::copy
#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>
#include <cassert>

#include <iomanip>

#include <cstdio>

// std::min
#include <cmath>

#include "FileSystem.h"
#include "File.h"

#define assertStream(stream) {              \
    assert( stream.is_open() );             \
    assert( stream.fail() == false );       \
    assert( stream.bad() == false );        \
}

void gotoBlock( uint64_t block , uint64_t blockSize , std::fstream &stream ) {
    assertStream( stream );

    stream.seekp( block * blockSize );
    stream.seekg( block * blockSize );

    assertStream( stream );
}

void printFile( os::File &file ) {
    std::cout << "File Properties" << std::endl;
    std::cout << "\tFile Name: " << file.name << std::endl;
    std::cout << "\tFile Start: " << file.start << std::endl;
    std::cout << "\tFile Current: " << file.current << std::endl;
    std::cout << "\tFile End: " << file.end << std::endl;
    std::cout << "\tFile Size: " << file.size << std::endl;
    std::cout << "\tFile Position: " << file.position << std::endl << std::endl;
}

void printBlock( os::Block &b ) {
    std::cout << "Printing Block:"  << std::endl;
    std::cout << "\tBlock: "        << b.block << std::endl;
    std::cout << "\tStatus: "        << ( (b.status == os::FULL)?"FULL":"LAZY") << std::endl;
    std::cout << "\tPrevious: "     << b.prev << std::endl;
    std::cout << "\tNext: "         << b.next << std::endl;
    std::cout << "\tLength: "       << b.length << std::endl << std::endl;
}

// Given a block and a start and end we remove all the bytes inbetween
void compact( os::Block &b, uint64_t start , uint64_t end ) {
    if( start >= end || start >= b.length ) return;

    // Remove trailing data
    if( end >= b.length ) {
        b.length = start;
        return;
    }


    uint64_t length = end - start;
    for( int i = 0 ; i < length; ++i ) {
        b.data[start + i] = b.data[end + i];
    }
    b.length -= length;
}


// namespace implementation

namespace os {

    // Private functions

    File FileSystem::createNewFile( std::string name ) {

        uint64_t junk;

        std::cout << "Creating a new file with the name " << name << std::endl;

        uint64_t lastFileBlock = 1;
        std::array<char,1024> buffer;
        char *reint;

        // Create file meta-data 
        //  File Meta-Data:
        //      length of name
        //      name
        //      first block
        //      last block
        //      size
        uint64_t lengthName = name.size();
        uint64_t numBytes = 4 * sizeof( uint64_t ) + lengthName;


        std::cout << "New file requires " << numBytes << " bytes of meta-data" << std::endl;

        metadataSize += numBytes;

        // Create data to write to the filesystem
        
        // Write length of name
        reint = reinterpret_cast<char*>( &lengthName );
        assert( lengthName == 4 );
        std::copy( reint , reint + sizeof(uint64_t) , buffer.begin() );

        // Write name
        std::copy( name.begin() , name.end()        , buffer.begin() + 1 * sizeof(uint64_t) );

        // Write first block
        lengthName = 0; 
        std::copy( reint , reint + sizeof(uint64_t) , buffer.begin() + 1 * sizeof(uint64_t) + name.size() );

        // Write last block
        std::copy( reint , reint + sizeof(uint64_t) , buffer.begin() + 2 * sizeof(uint64_t) + name.size() );

        // Write size
        std::copy( reint , reint + sizeof(uint64_t) , buffer.begin() + 3 * sizeof(uint64_t) + name.size() );


        // Write data to blocks
        gotoBlock( lastFileBlock );
        Block b = readBlock();

        uint64_t bytesToWrite = std::min( numBytes, (BlockSize - b.length) );
        std::copy( buffer.begin() , buffer.begin() + bytesToWrite , b.data.data() );
        b.length += bytesToWrite;
        numBytes -= bytesToWrite;
        if( numBytes > 0 ) {
            Block end = allocate( numBytes , buffer.begin() + bytesToWrite );
            b.next = end.block;
            end.prev = b.block;
            flush( end );
        }
        std::cout << "About to write this block to disk" << std::endl;
        printBlock( b );
        flush( b );

        // Update header
        ++numFiles;
        saveHeader();

        // Return new file
        File f;
        f.name = name;
        f.size = 0;
        f.start = 0;
        f.end = 0;
        f.position = 0;
        f.fs = this;

        return f;
    }

    void FileSystem::gotoBlock( uint64_t blockId ) {
        uint64_t offset = blockId * TotalBlockSize;

        std::cout << "Moving to block " << blockId << " at offset " << offset << std::endl;
        std::cout << std::endl;

        stream.seekp( blockId * TotalBlockSize );
        stream.seekg( blockId * TotalBlockSize );
    }

    Block FileSystem::readBlock( ) {
        Block ret;
        ret.status = FULL;
        ret.block = stream.tellp() / TotalBlockSize;

        std::cout << "About to read block " << ret.block << std::endl;
        assertStream( stream );

        lock( READ );
        {
            stream.read( reinterpret_cast<char*>( &ret.prev ) , sizeof( ret.prev ) );
            stream.read( reinterpret_cast<char*>( &ret.next ) , sizeof( ret.next ) );
            stream.read( reinterpret_cast<char*>( &ret.length ) , sizeof( ret.length ) );
            stream.read( ret.data.data() , ret.length );
        }
        unlock( READ );

        assertStream( stream );

        return ret;
    }

    void FileSystem::writeBlock( Block &b ) {

        std::cout << "Writing Block:" << std::endl;
        printBlock( b );


        assert( stream.tellp() == b.block * TotalBlockSize );

        assertStream( stream );

        lock( WRITE );
        {
            stream.write( reinterpret_cast<char*>( &(b.prev) ) , sizeof( b.prev ) );
            stream.write( reinterpret_cast<char*>( &(b.next) ) , sizeof( b.next ) );
            stream.write( reinterpret_cast<char*>( &(b.length) ) , sizeof( b.length ) );
            if( b.status == FULL ) {
                stream.write( b.data.data() , b.length );
            }
        }
        unlock( WRITE );

        assertStream( stream );

    }

    void FileSystem::saveHeader() {

        assertStream( stream );

        std::cout << "Saving header:" << std::endl;
        std::cout << "\tTotal number of bytes stored: "           << totalBytes << std::endl;
        std::cout << "\tFirst block of the free list: "           << freeList << std::endl;
        std::cout << "\tNumber of blocks used for file data: "    << numBlocks << std::endl;
        std::cout << "\tNumber of free blocks: "                  << numFreeBlocks << std::endl;
        std::cout << "\tNumber of files created: "                << numFiles << std::endl;
        std::cout << "\tAmount of data used for meta-data: "      << metadataSize << std::endl;

        stream.seekp( SignatureSize , std::ios_base::beg );

        assert( stream.tellp() == 8 );

        // Write header
        stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) );
        stream.write( reinterpret_cast<char*>(&freeList) , sizeof(freeList) );
        stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof(numBlocks) );
        stream.write( reinterpret_cast<char*>(&numFreeBlocks) , sizeof(numFreeBlocks) );
        stream.write( reinterpret_cast<char*>(&numFiles) , sizeof(numFiles) );
        stream.write( reinterpret_cast<char*>(&metadataSize) , sizeof(metadataSize) );
        stream.write( reinterpret_cast<char*>(&lastFileBlock) , sizeof(lastFileBlock) );
        stream.flush();

        assertStream( stream );

    }

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

        Block newBlock = allocate( b.length - offset , b.data.begin() + offset );
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
        uint64_t blocksToWrite = (bytes + BlockSize) / BlockSize;
        uint64_t current = numBlocks;

        const char Zero[BlockSize] = {0};
        if( buffer == NULL ) {
            buffer = Zero;
        }


        std::cout << "Growing filesystem by " << bytes << " bytes" << std::endl;
        std::cout << "Adding " << blocksToWrite << " block(s)" << std::endl << std::endl;

        // Just grow first
        lock( WRITE );
        {
            numBlocks += blocksToWrite;
            totalBytes += bytes;

            std::cout << "Total Bytes: " << numBlocks * TotalBlockSize << std::endl;
            assert( numBlocks * TotalBlockSize == 3072 );
            assertStream( stream );

            stream.seekp( numBlocks * TotalBlockSize , std::ios_base::beg );
            stream.write( Zero , 1 );
            stream.flush();

            saveHeader();
            assertStream( stream );
        }unlock( WRITE );

        assertStream( stream );

        // In case we have no data

        // Write new data
        stream.seekp( TotalBlockSize * current , std::ios_base::beg );

        uint64_t previous = numBlocks - 1;
        for( int i = 0 ; i < blocksToWrite - 1; ++i) {
            Block curr;
            curr.prev = previous;
            curr.block = current; 
            curr.next = current + 1;
            
            uint64_t bytesM = std::min( BlockSize , bytes );
            std::copy( buffer , buffer + BlockSize , curr.data.begin() );
            if( buffer != Zero ) buffer += BlockSize;
            writeBlock( curr );

            previous = current;
            ++current;
        }

        bytes = bytes % BlockSize;
        assert( bytes > 0 );

        Block curr;
        curr.status = FULL;
        curr.prev = (previous == current) ? 0 : previous;
        curr.block = current;
        curr.next = 0;
        curr.length = bytes;
        std::copy( buffer , buffer + bytes , curr.data.begin() );
        std::copy( Zero + bytes , Zero + BlockSize , curr.data.begin() + bytes );

        std::cout << std::endl << "Final block is " << std::endl;
        printBlock( curr );
        flush( curr );

        assert( ( numBlocks - blocksToWrite - 1) < 1000 );

        return load( numBlocks - blocksToWrite );
    }


    //  load   -   Given a block id we load it from disk if not already loaded
    // 
    //  block       -   Block id we want to load
    //
    //  @return     -   Loaded block
    //
    Block FileSystem::load( uint64_t block ) {
        assert( stream.bad() == false );
        assert( stream.fail() == false );
        assert( block < 100000 );

        gotoBlock( block );
        return readBlock();
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
        b.status = LAZY;
        b.block = block;
        gotoBlock( block );
        lock( READ );
        {
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
        gotoBlock( b.block );
        writeBlock( b );
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

        assertStream( stream );

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
                    std::copy( buffer , buffer + current.length , current.data.data() );
                    fillOffset = BlockSize - current.length;
                }
                std::fill( current.data.begin() + fillOffset , current.data.begin() + BlockSize , 0 );

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
    Block FileSystem::allocate( uint64_t length , const char *buffer ) {

        std::cout << "In Allocate" << std::endl;

        assertStream( stream );

        Block head;
        if( length == 0 ) {
            assert( false );
            return head;
        }

        if( numFreeBlocks > 0 ){
            assert( false );
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
    //  offset      -   How many bytes from the starting block, this is updated
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
    uint64_t FileSystem::read( File &file , uint64_t length , char* buffer ) {

        printFile( file );
        std::cout << "Reading " << length << " bytes" << std::endl;

        assertStream( stream );

        uint64_t requested = length;
        length = std::min( length , file.size - file.position );

        if( length > 0 && buffer != 0 && file.position < file.size ) {
            std::cout << "Here I am!" << std::endl;
            Block current = locate( file.current , file.position );

            printBlock( current );

            uint64_t next = current.block;
            current = load( next );

            printBlock( current );

            for( int i = 0 ; i < length ; ++i ) {
                buffer[i] = current.data[file.position];
                ++file.position;
                if( file.position == current.length ) {
                    current = load( current.next );
                    file.current = current.block;
                    file.position = 0;
                }
            }
        }else {
            assert( false );
        }
        // How much we read
        
        assertStream( stream );

        return requested - length;
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
    uint64_t FileSystem::write( File &file , uint64_t length , const char* buffer ) {
        uint64_t requested = length;

        assertStream( stream );

        printFile( file );

        assert( length > 0 );
        assert( buffer !=  0);

        // If we are about to write somewhere that does not exist
        // create enough space for it
        if( file.current >= file.end ) {
            Block remaining  = allocate( length , buffer );
            if( file.start == 0 ) {
                file.start = remaining.block;
                file.end = remaining.prev;
                file.current = remaining.block;
                file.position = length % BlockSize;
                flush( remaining );

                metadata->position = file.metadata + sizeof(uint64_t) + file.name.size();
                metadata->current = 0;

                write( *metadata , file.start , reinterpret_cast<char*>(&(file.start)));
                write( *metadata , file.end , reinterpret_cast<char*>(&(file.end)));
                write( *metadata , file.size , reinterpret_cast<char*>(&(file.size)));

                //file.flush();
            }else {
                Block last = lazyLoad( file.end );
                file.end = remaining.prev;
                file.current = file.end;

                last.next = remaining.block;
                remaining.prev = last.block;

                flush( last );
                flush( remaining );
                //file.flush();
            }
        }else {
            assert( false );
            assert( numFreeBlocks == 0 );
            if( length > 0 && buffer != 0 ) {
                Block current = locate( file.current , file.position );
                assert( current.block != 0 );
                uint64_t next = current.block;

                do {
                    // Go to next
                    current = load( next );

                    // Copy from buffer to file
                    uint64_t len = std::min( current.length - file.position , length );
                    std::copy( buffer , buffer + len , current.data.begin() + file.position );
                    assert( current.block != 0 );

                    // Update variables
                    file.position = 0;
                    buffer += len;
                    length -= len;

                    flush( current );

                    next = current.next;

                } while( length > 0 );

                // Grow the file
                if( requested > length ) {
                    Block remaining = allocate( length , buffer );
                    length = 0;
                    current.next = remaining.block;
                    file.end = remaining.prev;
                    remaining.prev = current.block;

                    flush( remaining );
                    flush( current );
                }

                file.current = current.block;
                file.position = length;

            }else {
                assert( false );
            }
        }
        // How much we read
        return requested - length;
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
    uint64_t FileSystem::insert( File &file , uint64_t length , const char* buffer ) {
        assert( numFreeBlocks == 0 );
        if( length > 0 ) {
            // Grow file if neccessary 
            if( file.position > file.size ) {
                // Actually allocate
                Block remaining = allocate( file.position - file.size , NULL );

                // Grab the current end of file block
                Block oldEnd = lazyLoad( file.end );

                // Update file
                file.end = remaining.prev;
                file.size += file.position - file.size;
                file.current = file.end;

                // Merge blocks together
                remaining.prev = oldEnd.block;
                oldEnd.next = remaining.block;

                flush( remaining );
                flush( oldEnd );
            }

            Block b = locate( file.current , file.position );

            // We might have to split
            split( b , file.position % BlockSize );

            // Now append to here
            Block next = allocate( length , buffer );
            Block end = lazyLoad( next.prev );
            end.next = b.next;
            next.prev = b.block;
            b.next = next.block;

            flush( next );
            flush( end );
            flush( b );

            // file.flush();
        }
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
        assert( numFreeBlocks == 0 );
        uint64_t remaining = std::min( length , file.size - file.position );

        // Make sure can remove bytes
        if( remaining > 0 && (file.size > file.position) ) {

            // Load first block we have to modify
            Block first = locate( file.start , file.position );

            // Calculate how much to remove from first
            uint64_t removeFromFirst = std::min( remaining , first.length - file.position );
            // and how many bytes to skip to get to new spot
            remaining -= removeFromFirst;

            // Load final block
            Block last = locate( first.next , remaining );
            last = load( last.block );

            // Compact blocks
            compact( first , file.current , removeFromFirst );
            compact( last , 0 , remaining );

            // Need to reconnect
            if( first.next != last.block ) {
                releaseBlock( first.next );
                first.next = last.block;
                last.prev = first.block;
            }

            // Update file
            file.size -= length - remaining;
            file.current = last.block;
            file.position = 0;

            // Write to disk
            flush( first );
            flush( last );

            // file.flush();
        }
        return length - remaining;

    }


    // Public Methods/Functions

    void FileSystem::shutdown() {
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ) {
            //(*file).close();
            openFiles.erase(file);
        }
        stream.close();
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
                f = *file;
                printFile( f );
                goto theend;
            }
        } 

        // Create new file
        f = createNewFile( name );
theend:
        return f;
    }

    bool FileSystem::unlink( File f ) {
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
        std::array<char,1024> buff;

        fileSystemLocation = filename;

        //  Open the file
        stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

        if( !stream ) {
            stream.open( filename , std::fstream::binary | std::fstream::trunc | std::fstream::out );
            stream.close();
            stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

            std::cout << "The file does not exist, so we are creating it" << std::endl;
            totalBytes = 0;
            freeList = 0;
            numBlocks = 2;
            numFreeBlocks = 0;
            numFiles = 0;
            metadataSize = 0;
            lastFileBlock = 1;

            // Now we go to the beginning
            stream.seekp( 0 , std::ios_base::beg );

            // Write header
            stream.write( HeaderSignature.data() , SignatureSize );
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) );
            stream.write( reinterpret_cast<char*>(&freeList) , sizeof(freeList) );
            stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof(numBlocks) );
            stream.write( reinterpret_cast<char*>(&numFreeBlocks) , sizeof(numFreeBlocks) );
            stream.write( reinterpret_cast<char*>(&numFiles) , sizeof(numFiles) );
            stream.write( reinterpret_cast<char*>(&metadataSize) , sizeof(metadataSize) );
            stream.write( reinterpret_cast<char*>(&lastFileBlock) , sizeof(lastFileBlock) );

            assert( TotalBlockSize == 1024 );
            stream.seekp( TotalBlockSize , std::ios_base::beg );

            // Write file "data-strcture"
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) ); // Prev
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) );  // Next
            stream.seekp( TotalBlockSize * 2 - 1 );
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ));

            stream.flush();

        }else {

            //  Read the header
            stream.read( buff.data() , SignatureSize );

            if( std::strncmp( buff.data() , HeaderSignature.data() , SignatureSize ) != 0 ) {
                std::cout << "Invalid header signature found" << std::endl;
                std::exit( -1 );
            }

            stream.read( reinterpret_cast<char*>(&totalBytes)       , sizeof(totalBytes) );
            stream.read( reinterpret_cast<char*>(&freeList)         , sizeof(freeList) );
            stream.read( reinterpret_cast<char*>(&numBlocks)        , sizeof(numBlocks) );
            stream.read( reinterpret_cast<char*>(&numFreeBlocks)    , sizeof(numFreeBlocks) );
            stream.read( reinterpret_cast<char*>(&numFiles)         , sizeof(numFiles) );
            stream.read( reinterpret_cast<char*>(&metadataSize)     , sizeof(metadataSize) );
            stream.read( reinterpret_cast<char*>(&lastFileBlock)     , sizeof(lastFileBlock) );

        }

        std::cout << "Total number of bytes stored: "           << totalBytes << std::endl;
        std::cout << "First block of the free list: "           << freeList << std::endl;
        std::cout << "Number of blocks used for file data: "    << numBlocks << std::endl;
        std::cout << "Number of free blocks: "                  << numFreeBlocks << std::endl;
        std::cout << "Number of files created: "                << numFiles << std::endl;
        std::cout << "Amount of data used for meta-data: "      << metadataSize << std::endl;

        //  Read the filenames
        std::cout << "Loading File Meta-data (" << numFiles << " files)" << std::endl;

        metadata = new File();

        metadata->name = "Metadata";
        metadata->start = 1;
        metadata->current = 1;
        metadata->end = lastFileBlock;
        metadata->size = metadataSize;
        metadata->position = 0;

        printFile( *metadata );
        for( int i = 0 ; i < numFiles; ++i) {
            File f;
            f.metadata = metadata->position;
            uint64_t str_len; 
            read( *metadata , sizeof(uint64_t) , reinterpret_cast<char*>(&str_len) );
            std::cout << "Name length is " << str_len << std::endl;
            read( *metadata , str_len , buff.data() );
            f.name = std::string( buff.data() , str_len );
            read( *metadata , sizeof(uint64_t) , reinterpret_cast<char*>(&(f.start)) );
            read( *metadata , sizeof(uint64_t) , reinterpret_cast<char*>(&(f.end)) );
            read( *metadata , sizeof(uint64_t) , reinterpret_cast<char*>(&(f.size)) );

            allFiles.push_back( f );

        }

        std::cout << "Finished reading Meta-data" << std::endl;

        assertStream( stream );
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
        delete metadata;
    }

};
