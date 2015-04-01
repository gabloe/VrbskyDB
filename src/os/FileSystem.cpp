// std::copy
#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>
#include <assert.h>

#include <cstdio>

// std::min
#include <cmath>

#include "FileSystem.h"
#include "File.h"


void printFile( os::File &file ) {
    std::cout << "File Properties" << std::endl;
    std::cout << "\tFile Name: " << file.name << std::endl;
    std::cout << "\tFile Start: " << file.start << std::endl;
    std::cout << "\tFile Current: " << file.current << std::endl;
    std::cout << "\tFile End: " << file.end << std::endl;
    std::cout << "\tFile Size: " << file.size << std::endl;
    std::cout << "\tFile Position: " << file.position << std::endl << std::endl;
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
        std::cout << "Creating a new file with the name " << name << std::endl;

        uint64_t lastFileBlock = 1;

        // Create file meta-data 
        uint64_t lengthName = name.size();
        uint64_t numBytes = sizeof( uint64_t ) + lengthName + sizeof( uint64_t );
        char buffer[1024];

        char *t = reinterpret_cast<char*>( &lengthName );
        std::copy( t , t + sizeof(uint64_t) , buffer );
        std::copy( name.begin() , name.end() , buffer + sizeof(uint64_t) );
        t = reinterpret_cast<char*>( &lastFileBlock );
        std::copy( t , t + sizeof(uint64_t) , buffer + sizeof(uint64_t) + lengthName );

        File f = File();
        f.name = "MetaData";
        f.size = metadataSize;
        f.start = 1;
        f.current = 1;
        f.end = lastFileBlock;
        f.position = metadataSize;

        // Write new file information 
        write( f , numBytes , buffer );

        // Update header
        metadataSize += numBytes;
        ++numFiles;
        saveHeader();

        // Return new file
        f.name = name;
        f.size = 0;
        f.start = 0;
        f.end = 0;
        f.position = 0;

        return f;
    }

    void FileSystem::saveHeader() {
        stream.seekp( SignatureSize , std::ios_base::beg );

        // Write header
        stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ) );
        stream.write( reinterpret_cast<char*>(&freeList) , sizeof( freeList ) );
        stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof( numBlocks) );
        stream.write( reinterpret_cast<char*>(&numFreeBlocks) , sizeof( numFreeBlocks ) );
        stream.write( reinterpret_cast<char*>(&numFiles) , sizeof( numFiles ) );
        stream.write( reinterpret_cast<char*>(&metadataSize) , sizeof( metadataSize ) );
        stream.flush();

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
            if( buffer != NULL ) {
                b.data[i] = buffer[i];
            }else {
                b.data[i] = 0;
            }
        }

        uint64_t previous = b.prev;
        // Begin our writing
        std::fstream secondary( fileSystemLocation, std::fstream::in | std::fstream::out | std::fstream::binary );

        secondary.seekp( HeaderSize + TotalBlockSize * blockId , std::ios_base::beg );
        for( int i = 0 ; i < blocksToWrite - 1; ++i) {
            secondary.write( reinterpret_cast<char*>( &previous ) , sizeof(uint64_t) ); // Prev 
            secondary.write( reinterpret_cast<char*>( &blockId ) , sizeof(uint64_t) ); // Next 
            secondary.write( reinterpret_cast<const char*>( &BlockSize ) , sizeof(uint64_t) ); // Length
            if( buffer != NULL ) {
                secondary.write( reinterpret_cast<const char*>( buffer ) , BlockSize ); // Data 
            }else {
                stream.seekp( BlockSize , std::ios_base::beg );
            }
            if( buffer != NULL ) {
                buffer += BlockSize;
            }
            previous = blockId;
            ++blockId;
        }

        uint64_t zero = 0;
        bytes = bytes % BlockSize;
        secondary.write( reinterpret_cast<char*>( &previous ) , sizeof(uint64_t) ); // Prev 
        secondary.write( reinterpret_cast<char*>( &zero ) , sizeof(uint64_t) ); // Next 
        secondary.write( reinterpret_cast<char*>( &bytes ) , sizeof(uint64_t) ); // Length
        if( buffer != NULL ) {
            secondary.write( reinterpret_cast<const char*>( buffer ) , bytes % BlockSize ); // Data 
        }
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
        std::cout << "Block: " << block << std::endl;
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

        assert( numFreeBlocks == 0 );

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
        uint64_t requested = length;
        length = std::min( length , file.size - file.position );

        if( length > 0 && buffer != 0 && file.position < file.size ) {
            Block current = locate( file.current , file.position );
            uint64_t next = current.block;

            do {
                // Go to next
                current = load( next );

                // Copy to buffer
                uint64_t len = std::min( current.length - file.position , length );
                std::copy( current.data + file.position , current.data + len , buffer );

                // Update variables
                file.position = 0;
                buffer += len;
                length -= len;

                next = current.next;

            }while( length > 0 );

            file.current = current.block;
            file.position = length;

        }
        // How much we read
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
            if( length > 0 && buffer != 0 ) {
                Block current = locate( file.current , file.position );
                assert( current.block != 0 );
                uint64_t next = current.block;

                do {
                    // Go to next
                    current = load( next );

                    // Copy from buffer to file
                    uint64_t len = std::min( current.length - file.position , length );
                    std::copy( buffer , buffer + len , current.data + file.position );
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
        // This file does not exist, we need to create
        // it and save it to disk.
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
            numBlocks = 0;
            numFreeBlocks = 0;
            numFiles = 0;
            metadataSize = 0;

            // Allocate the iniial space
            //grow( TotalBlockSize * 2 , NULL );

            // Now we go to the beginning
            stream.seekp( 0 , std::ios_base::beg );

            // Write header
            stream.write( HeaderSignature , SignatureSize );
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ) );
            stream.write( reinterpret_cast<char*>(&freeList) , sizeof( freeList ) );
            stream.write( reinterpret_cast<char*>(&numBlocks) , sizeof( numBlocks) );
            stream.write( reinterpret_cast<char*>(&numFreeBlocks) , sizeof( numFreeBlocks ) );
            stream.write( reinterpret_cast<char*>(&numFiles) , sizeof( numFiles ) );
            stream.write( reinterpret_cast<char*>(&metadataSize) , sizeof( metadataSize ) );
            assert( TotalBlockSize == 1024 );
            stream.seekp( TotalBlockSize , std::ios_base::beg );

            // Write file "data-strcture"
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) ); // Prev
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof(totalBytes) );  // Next
            stream.seekp( TotalBlockSize * 2 - 1 );
            stream.write( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ));

            assert( numFreeBlocks == 0 );
            stream.flush();

        }else {

            //  Read the header
            char buff[BlockSize];
            stream.read( buff , SignatureSize );

            if( std::strncmp( buff , HeaderSignature , SignatureSize ) != 0 ) {
                std::cout << "Invalid header signature found" << std::endl;
                std::exit( -1 );
            }
            stream.read( reinterpret_cast<char*>(&totalBytes) , sizeof( totalBytes ) );
            stream.read( reinterpret_cast<char*>(&freeList) , sizeof( freeList ) );
            stream.read( reinterpret_cast<char*>(&numBlocks) , sizeof( numBlocks) );
            stream.read( reinterpret_cast<char*>(&numFreeBlocks) , sizeof( numFreeBlocks ) );
            stream.read( reinterpret_cast<char*>(&numFiles) , sizeof( numFiles ) );
            stream.read( reinterpret_cast<char*>(&metadataSize) , sizeof( metadataSize ) );


            std::cout << "Total number of bytes stored: "           << totalBytes << std::endl;
            std::cout << "First block of the free list: "           << freeList << std::endl;
            std::cout << "Number of blocks used for file data: "    << numBlocks << std::endl;
            std::cout << "Number of free blocks: "                  << numFreeBlocks << std::endl;
            std::cout << "Number of files created: "                << numFiles << std::endl;
            std::cout << "Amount of data used for meta-data: "      << metadataSize << std::endl;

            //  Read the filenames
            Block b = load( 1 );
            for( int i = 0 ; i < numFiles; ++i) {
                File f;

                // First is the length of the filename
                uint64_t str_len = 0;
                stream.read( reinterpret_cast<char*>(&str_len) , sizeof(str_len) );

                // Read the filename
                stream.read( buff , str_len );
                f.name = std::string( buff , str_len );

                // Read the block
                stream.read( reinterpret_cast<char*>(&str_len) , sizeof(str_len) );

                // Save the file
                allFiles.push_back( f );
            }

        }
        std::cout << "Derping" << std::endl;
        assert( numFreeBlocks == 0 );

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
