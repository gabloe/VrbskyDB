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

#include "Logging.h"
#include "FileSystem.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "File.h"

static const char Zero[os::Block_Size] = {0};

static Log l(std::cout);

#define assertStream(stream) {              \
    assert( stream );                       \
    assert( stream.is_open() );             \
    assert( stream.fail() == false );       \
    assert( stream.bad() == false );        \
}


void write64( std::fstream& str , uint64_t t ) {
    str.write( reinterpret_cast<char*>( &t ) , sizeof(t) );
}

void read64( std::fstream& str , uint64_t &t ) {
    str.read( reinterpret_cast<char*>( &t ) , sizeof(t) );
}

void gotoBlock( uint64_t block , uint64_t blockSize , std::fstream &stream ) {
    assertStream( stream );

    stream.seekp( block * blockSize );
    stream.seekg( block * blockSize );

    assertStream( stream );
}

void printFile( os::File &file , bool force = false ) {
    l.log( "File Properties"      , force );
    l.log( "\tFile Name"          , file.name             , force );
    l.log( "\tFile Size"          , file.size             , force );
    l.log( "\tFile Start"         , file.start            , force );
    l.log( "\tFile Current"       , file.current          , force );
    l.log( "\tFile Position"      , file.position         , force ); 
    l.log( "\tBlock Position"     , file.block_position   , force );
    l.log( "\tDisk Position"      , file.disk_position    , force );
    l.log( "\tFile End"           , file.end              , force );
    l.log( "\tMeta-data position" , file.metadata         , force ); 
}

void printBlock( os::Block &b , bool force = false ) {
    l.log( "Printing Block" , force );
    l.log( "\tBlock"        , b.block , force );
    l.log( "\tStatus"       , ( (b.status == os::FULL)?"FULL":"LAZY") , force );
    l.log( "\tPrevious"     , b.prev , force );
    l.log( "\tNext"         , b.next , force );
    l.log( "\tLength"       , b.length , force );
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
    uint64_t FileSystem::insertFile( File &f ) {
        l.enter("INSERTFILE");

        std::array<char,1024> buffer;

        // Length of name
        // name
        // disk usage
        // size
        // first block
        // last block
        // metadata
        
        uint64_t pos  = 0;
        uint64_t tmp= 0;
        char *start = reinterpret_cast<char*>(&tmp);
        char *end = start + sizeof(tmp);

        printFile( f , true );

        tmp = f.name.size();
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        std::copy(f.name.begin(), f.name.end(), buffer.begin() + pos);
        pos += f.name.size();

        tmp = f.disk_usage;
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        tmp = f.size;
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        tmp = f.start;
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        tmp = f.end;
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        tmp = f.metadata;
        std::copy(start, end, buffer.begin() + pos);
        pos += sizeof(tmp);

        metaWriter->seek( f.metadata , BEG );
        assert( metadata->position == f.metadata );
        metaWriter->write( pos , buffer.data() );

        l.leave("INSERTFILE");
        return pos;

    }

    File& FileSystem::createNewFile( std::string name ) {
        l.enter( "CREATENEWFILE" );

        Block b = allocate( Block_Size , NULL );

        File &f = *(new File());
        f.name = name;
        f.size = 0;
        f.disk_usage = Block_Size;
        f.start = f.end = f.current = b.block;
        f.position = f.block_position = f.disk_position = 0;
        f.metadata = metadata->size;
        f.fs = this;

        metadata_bytes_used += insertFile( f );
        metadata_files++;
        saveHeader();

        l.leave( "CREATENEWFILE" );
        return f;
    }

    void FileSystem::gotoBlock( uint64_t blockId ) {
        assert( blockId > 0 );
        assert( blockId < blocks_allocated );
        l.enter( "GOTOBLOCK" );
        uint64_t offset = blockId * Total_Size_Block;

        l.log( "Moving to block" , blockId );
        l.log( "At offset" , offset );

        stream.seekp( blockId * Total_Size_Block );
        stream.seekg( blockId * Total_Size_Block );
        l.leave( "GOTOBLOCK" );
    }

    Block FileSystem::readBlock( ) {
        l.enter( "READBLOCK" );
        Block ret;
        ret.status = FULL;
        ret.block = stream.tellp() / Total_Size_Block;

        l.log( "About to read block" , ret.block );
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
        l.leave( "READBLOCK" );

        return ret;
    }

    void FileSystem::writeBlock( Block &b ) {
        l.enter( "WRITEBLOCK" );

        assert( stream.tellp() == b.block * Total_Size_Block );
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
        l.leave( "WRITEBLOCK" );

    }

    void FileSystem::loadMetadata() {
        std::array<char,1024> buff;
        l.enter("LOADMETADATA");

        metadata = new File();
        metaWriter = new FileWriter( *metadata );
        metaReader = new FileReader( *metadata );
        metadata->fs = this;
        metadata->name = "Metadata";
        metadata->start = 1;
        metadata->current = 1;
        metadata->end = metadata_end;
        metadata->size = metadata_bytes_used;
        metadata->disk_usage = Block_Size;

        if( metadata_files > 0 ) {

            metaReader->seek( 0 , BEG );
            assert( metadata->position == 0 );
            assert( metaReader->file.position == 0 );

            for( int i = 0 ; i < metadata_files; ++i) {
                File &f = *(new File());

                l.disableMethod();
                uint64_t str_len,f_position = metaReader->tell(); 
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&str_len) );
                l.log( "str_len" , str_len );
                metaReader->read( str_len , buff.data() );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.disk_usage)) );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.size)));
                l.log( "Size" , f.size , true );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.start)));
                l.log( "Start" , f.start , true );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.end)));
                l.log( "End" , f.end , true );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.metadata)));
                l.log( "Meta" , f.metadata , true );
                l.enableMethod();

                f.fs = this;
                f.name = std::string( buff.data() , str_len );
                f.current = f.start;

                printFile( f , true );

                allFiles.push_back( &f );
            }
        }
        l.leave("LOADMETADATA");
    }

    void FileSystem::printHeader() {
        l.log( "Bytes Allocated", bytes_allocated , true );
        l.log( "Bytes Used", bytes_used , true );
        l.log( "Blocks Allocated", blocks_allocated , true );
        l.log( "Blocks Used", blocks_used , true );
        l.log( "Number free blocks", free_count , true );
        l.log( "First free block", free_first , true );
        l.log( "Bytes allocated for metadata", metadata_bytes_allocated , true );
        l.log( "Blocks allocated for metadata", metadata_allocated_blocks , true );
        l.log( "Bytes used for meta-data", metadata_bytes_used , true );
        l.log( "Blocks used for metadata", metadata_blocks_used , true );
        l.log( "Files created", metadata_files , true );
        l.log( "First block for metadata", metadata_start , true );
        l.log( "Last block for metadata", metadata_end , true );
    }

    void FileSystem::readHeader() {
        stream.seekp( SignatureSize , std::ios_base::beg );
        assert( stream.tellp() == 8 );
        read64( stream , bytes_allocated );
        read64( stream , bytes_used );
        read64( stream , blocks_allocated );
        read64( stream , blocks_used );
        read64( stream , free_count );
        read64( stream , free_first );
        read64( stream , metadata_bytes_allocated );
        read64( stream , metadata_allocated_blocks );
        read64( stream , metadata_bytes_used );
        read64( stream , metadata_blocks_used );
        read64( stream , metadata_files );
        read64( stream , metadata_start );
        read64( stream , metadata_end );

    }

    void FileSystem::saveHeader() {
        l.enter( "SAVEHEADER" );

        assertStream( stream );

        stream.seekp( SignatureSize , std::ios_base::beg );
        assert( stream.tellp() == 8 );
        write64( stream , bytes_allocated );
        write64( stream , bytes_used );
        write64( stream , blocks_allocated );
        write64( stream , blocks_used );
        write64( stream , free_count );
        write64( stream , free_first );
        write64( stream , metadata_bytes_allocated );
        write64( stream , metadata_allocated_blocks );
        write64( stream , metadata_bytes_used );
        write64( stream , metadata_blocks_used );
        write64( stream , metadata_files );
        write64( stream , metadata_start );
        write64( stream , metadata_end );

        stream.flush();

        assertStream( stream );
        l.leave( "SAVEHEADER" );

    }

    /*
     *  Locking Functions
     */

    // Lock the file for writing
    void FileSystem::lock( LockType type ){
        //l.enter( "LOCK" );
        switch( type ) {
            case READ:
                break;
            case WRITE:
                break;
        }
        //l.leave( "LOCK" );
    }

    void FileSystem::unlock( LockType type ) {
        //l.enter( "UNLOCK" );
        switch( type ) {
            case READ:
                break;
            case WRITE:
                break;
        }
        //l.leave( "UNLOCK" );
    }


    /*
     *  Low level filesystem functions
     */

    bool FileSystem::rename( File &toRename , const std::string newName ) {
        l.enter( "RENAME" );
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
            File &f = *(*file);
            if( f.getFilename() == toRename.getFilename() ) {
                f.name = newName;
                toRename.name = newName;
                return true;
            }
        }
        l.leave( "RENAME" );
        return false;
    }


    void FileSystem::split( Block &b , uint64_t offset ) {
        l.enter( "SPLIT" );
        if( offset != b.length ) {

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
        l.leave( "SPLIT" );

    }

    //  grow    -   Grow the filesystem enough to support writing the number of bytes given
    //
    //  bytes   -   How much space we need
    //
    //  @return -   The first block of the new space
    //
    Block FileSystem::grow( uint64_t bytes , const char *buffer ) {
        l.enter( "GROW" );

        uint64_t blocks_to_write = (bytes + Block_Size - 1) / Block_Size;
        uint64_t current = blocks_allocated;

        if( buffer == NULL ) {
            buffer = Zero;
        }

        lock( WRITE );
        {
            blocks_allocated += blocks_to_write;
            bytes_allocated += bytes;

            stream.seekp( Total_Size_Block * blocks_allocated - 1 , std::ios_base::beg );
            stream.write( Zero , 1 );
            stream.flush();

            saveHeader();
            assertStream( stream );
        }unlock( WRITE );

        // Write new data
        stream.seekp( Total_Size_Block * current , std::ios_base::beg );

        assert( current != 0 );

        uint64_t previous = blocks_allocated - 1;
        for( int i = 0 ; i < blocks_to_write - 1; ++i) {
            Block curr;
            curr.prev = previous;
            curr.block = current; 
            curr.next = current + 1;

            uint64_t bytesM = std::min( Block_Size , bytes );
            std::copy( buffer , buffer + Block_Size , curr.data.begin() );
            if( buffer != Zero ) {
                buffer += Block_Size;
                curr.length = Block_Size;
            }
            writeBlock( curr );

            previous = current;
            ++current;
            bytes -= bytesM;
        }

        assert( bytes > 0 );

        Block curr;
        curr.status = FULL;
        curr.prev = (previous == current) ? 0 : previous;
        curr.block = current;
        curr.next = 0;
        if( buffer == Zero ) {
            curr.length = 0;
        }else {
        curr.length = bytes;
        }
        std::copy( buffer , buffer + bytes , curr.data.begin() );
        std::copy( Zero + bytes , Zero + Block_Size , curr.data.begin() + bytes );

        flush( curr );

        curr = load( blocks_allocated - blocks_to_write );

        assert( ( blocks_allocated - blocks_to_write - 1) < 1000 );
        l.leave( "GROW" );

        return curr;
    }


    //  load   -   Given a block id we load it from disk if not already loaded
    // 
    //  block       -   Block id we want to load
    //
    //  @return     -   Loaded block
    //
    Block FileSystem::load( uint64_t block ) {
        l.enter( "LOAD" );
        assert( stream.bad() == false );
        assert( stream.fail() == false );
        assert( block < 10 );
        assert( block > 0 );

        gotoBlock( block );
        Block b = readBlock();

        l.leave( "LOAD" );

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
        l.enter( "LAZYLOAD" );
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

        l.leave( "LAZYLOAD" );

        return b;
    }

    //  flush   -   Given a block we flush it to disk
    //
    //  b       -   The block, with its data, we want to save to disk
    //  
    //
    void FileSystem::flush( Block &b ) {
        l.enter( "FLUSH" );
        gotoBlock( b.block );
        writeBlock( b );
        l.leave( "FLUSH" );
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
        l.enter( "REUSE" );
        Block head;

        assertStream( stream );

        if( free_count > 0 ) {
            uint64_t prevBlock = 0,currBlock = free_first;
            Block current;

            head = lazyLoad( currBlock );

            do {
                // Update free list
                free_first = current.next;
                --free_count;

                // Currently we treat the free_first as a special file, so just use it as such
                current = lazyLoad( currBlock );
                current.status = FULL;
                current.length = std::min( length , Block_Size );
                uint64_t fillOffset = 0;
                if( buffer != 0 ) {
                    std::copy( buffer , buffer + current.length , current.data.data() );
                    fillOffset = Block_Size - current.length;
                }
                std::fill( current.data.begin() + fillOffset , current.data.begin() + Block_Size , 0 );

                // Update input
                buffer += current.length;
                length -= current.length;

                // Write to disk
                flush( current );

                // Go to next
                prevBlock = currBlock;
                currBlock = free_first;

            } while ( free_count > 0 && length > 0 );

            current.next = 0;
            head.prev = prevBlock;
            flush( current );
            flush( head );
        }

        l.leave( "REUSE" );
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
        l.enter( "ALLOCATE" );
        assertStream( stream );

        Block head;
        if( length == 0 ) {
            assert( length != 0 );
            return head;
        }

        if( free_count > 0 ){
            assert( free_count == 0 );
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
        l.leave( "ALLOCATE" );
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
        l.enter( "LOCATE" );

        Block b = lazyLoad( start );
        while( offset >= b.length ) {
            offset -= b.length;
            if( b.next == 0 ) {
                b = Block();
                break;
            }
            b = lazyLoad( b.next );
        }
        l.leave( "LOCATE" );
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
        l.enter( "READ" );

        printFile( file , true );

        assertStream( stream );

        uint64_t requested = length;
        length = std::min( length , file.size - file.position );

        if( length > 0 && buffer != 0 && file.position < file.size ) {
            Block current = load( file.current );
            assert( file.position < 1000 );

            uint64_t next = current.block;
            current = load( next );

            for( int i = 0 ; i < length ; ++i ) {
                buffer[i] = current.data[file.block_position];
                ++file.position;
                ++file.block_position;
                ++file.disk_position;
                if( file.block_position  == current.length ) {
                    printBlock( current , true );
                    if( current.next != 0 ) {
                        file.disk_position += Block_Size - current.length;
                        current = load( current.next );
                        file.block_position = 0;
                        file.current = current.block;
                    }else {
                        break;
                    }
                }
            }
        }else {
            assert( false );
        }
        // How much we read

        assertStream( stream );
        l.leave( "READ" );

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
        l.enter( "WRITE" );

        l.log( "Filename" , file.name , true );
        l.log( "File Position" , file.position , true );
        l.log( "File Size" , file.size , true );
        l.log( "Length to write" , length , true );

        // Going to fill up rest of the file
        if( length + file.disk_position > file.disk_usage ) {
            // Calculate how much extra space we need
            uint64_t growBy = length - (file.disk_usage - file.disk_position);

            // Load for reconnection
            Block oldBlock = lazyLoad( file.end );
            Block newBlock = grow( growBy , NULL );

            // Update/Reconnect
            file.disk_usage += round<Block_Size>( growBy );
            file.end = newBlock.prev;
            oldBlock.next = newBlock.block;
            newBlock.prev = oldBlock.block;
            flush( newBlock );
            flush( oldBlock );

            // Save file data
            file.disk_usage += growBy; 
            bytes_allocated += growBy;

        }

        // We assume we always have space to write to

        uint64_t curr = file.current;
        uint64_t written = 0;               // How much data we have written
        uint64_t overwritten = 0;           // How much data we have overwritten
        uint64_t to_overwrite = 0;          // How much left we need to remove

        if( file.position < file.size ) {
            to_overwrite = std::min( length , file.size - file.position );
        }

        // While we have not written everything
        while( written < length ) {

            l.log( "In the loop" , true );

            // Go to the current block
            file.current = curr;
            Block b = load( curr );

            // How much room is there to write our data
            uint64_t len = std::min( Block_Size - file.block_position , length );
            l.log( "Length is" , len , true );
            // How much data will we actually overwrite
            overwritten += b.length + file.block_position;

            std::copy( buffer + written , buffer + len + written , b.data.data() + file.block_position );
            b.length = file.block_position + len;
            assert( b.status == FULL );
            flush( b );

            written += len;

            file.position += len;
            file.block_position = b.length % Block_Size;
            file.disk_position += len;

            curr = b.next;
        }


        // Need to move blocks around
        if( overwritten < to_overwrite ) {
            l.log( overwritten , to_overwrite , true );
            assert( false && "TODO: Handle case 'overwritten < length'"  );
        }else if( file.position > file.size  ) {
            l.log( "File has grown" , file.position - file.size , true );
            bytes_used += file.position - file.size;
            file.size += file.position - file.size;
            bytes_used += file.position - file.size;;
        }

        // TODO: What happens if we fill up file exactly to end?

        if( &file != metadata ) { 
            insertFile( file );
        }

        l.leave( "WRITE" );
        return written;
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
        l.enter( "INSERT" );
        assert( free_count == 0 );
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
            split( b , file.position % Block_Size );

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
        l.leave( "INSERT" );
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
        l.enter( "REMOVE" );
        assert( free_count == 0 );
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
        l.leave( "REMOVE" );
        return length - remaining;

    }


    // Public Methods/Functions

    void FileSystem::shutdown() {
        l.enter( "SHUTDOWN" );
        static bool done = false;
        if( !done ) { 
            done = true;
            saveHeader();
            for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
                l.log( "Closing a file" , true );
                if( (*file)->status == OPEN ) {
                    insertFile( **file );
                    (*file)->status = CLOSED;
                }
            }
            stream.flush();
            stream.close();
        }
        l.leave( "SHUTDOWN" );
    }

    std::list<File*> FileSystem::getFiles() {
        l.enter( "GETFILES" );
        std::list<File*> ret;
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            ret.push_back(*file);
        }
        l.leave( "GETFILES" );
        return ret;
    }

    std::list<File*> FileSystem::getOpenFiles() {
        l.enter( "GETOPENFILES" );
        std::list<File*> ret;
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
            ret.push_back(*file);
        }
        return ret;
        l.leave( "GETOPENFILES" );
    }

    std::list<std::string> FileSystem::getFilenames() {
        l.enter( "GETFILENAMES" );
        std::list<std::string> names;
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            names.push_back( (*file)->getFilename() );
        } 
        l.leave( "GETFILENAMES" );
        return names;
    }

    File& FileSystem::open( const std::string name) {
        l.enter( "OPEN" );
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            if( (*file)->getFilename() == name ) {
                openFiles.push_back( *file );
                (*file)->status = OPEN;
                l.leave("OPEN");
                return **file;
            }
        } 
        // Create new file
        File &f = createNewFile( name );
        f.status = OPEN;
        openFiles.push_back( &f );
        l.leave( "OPEN" );
        return f;
    }

    bool FileSystem::unlink( File &f ) {
        l.enter( "UNLINK:1" );
        l.leave( "UNLINK:1" );
    }

    //
    bool FileSystem::unlink( const std::string filename ) {
        l.enter( "UNLINK:2" );
        File& file = open(filename);
        l.leave( "UNLINK:2" );
        return file.unlink();
    }

    bool FileSystem::exists( const std::string filename ) {
        l.enter( "EXISTS" );
        File& file = open(filename);
        l.leave( "EXISTS" );
        return file.getStatus() == OPEN;
    }


    FileSystem::FileSystem( const std::string filename) {
        l.toggleOutput();
        l.enter( "FILESYSTEM" );

        fileSystemLocation = filename;

        //  Open the file
        stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

        if( !stream ) {
            stream.open( filename , std::fstream::binary | std::fstream::trunc | std::fstream::out );
            stream.close();
            stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

            l.log( "The file does not exist, so we are creating it" );

            // Make room for everything
            stream.seekp( Total_Size_Block * 2 - 1 );
            stream.write( Zero , 1 );
            stream.seekp( 0 );

            // Write Header
            stream.write( HeaderSignature.data() , SignatureSize );
            saveHeader();

            // Write FileSystem meta-data
            gotoBlock( 1 );
            Block b;
            b.block = 1;
            b.prev = b.next = b.length = 0;
            writeBlock( b );


        }else {

            //  Read the header
            std::array<char,SignatureSize> buff;
            stream.read( buff.data() , SignatureSize );

            if( std::strncmp( buff.data() , HeaderSignature.data() , SignatureSize ) != 0 ) {
                std::cout << "Invalid header signature found" << std::endl;
                std::exit( -1 );
            }
            readHeader();
        }
        printHeader();

        loadMetadata();


        l.leave( "FILESYSTEM" );
        assertStream( stream );
    }

    void FileSystem::closing( File& fp ) {
        l.enter( "CLOSING" );
        for( auto file = openFiles.begin(); file != openFiles.end(); ++file ) {
            File &f = *(*file);
            if( f.getFilename() == fp.getFilename() ) {
                openFiles.erase(file);
                break;
            }
        }
        l.leave( "CLOSING" );
    }

    FileSystem::~FileSystem() {
        l.enter( "~FILESYSTEM" );
        printHeader();
        shutdown();
        delete metaWriter;
        delete metaReader;
        delete metadata;
        l.leave( "~FILESYSTEM" );
    }

};
