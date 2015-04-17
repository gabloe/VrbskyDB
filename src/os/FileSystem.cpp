// std::copy
#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>

#include <iomanip>

#include <cstdio>

// std::min
#include <cmath>

#include "../os/Logging.h"
#include "../os/FileSystem.h"
#include "../os/FileReader.h"
#include "../os/FileWriter.h"
#include "../os/File.h"
#include "../assert/Assert.h"

static const char Zero[os::Block_Size] = {0};

static Log l(std::cout);

#define assertStream(stream) {                          \
    Assert( "Stream is not good", stream.good() );      \
    Assert( "Stream is not open", stream.is_open() );   \
    Assert( "Stream failed" , stream.fail() == false ); \
    Assert( "Stream is bad" , stream.bad() == false );  \
}


void write64( std::fstream& str , uint64_t t ) {
    str.write( reinterpret_cast<char*>( &t ) , sizeof(t) );
}

void read64( std::fstream& str , uint64_t &t ) {
    str.read( reinterpret_cast<char*>( &t ) , sizeof(t) );
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
        metaWriter->write( pos , buffer.data() );

        l.leave("INSERTFILE");
        return pos;

    }

    File& FileSystem::createNewFile( const std::string &name ) {
        l.enter( "CREATENEWFILE" );

        Block &b = *allocate( Block_Size , NULL );
        Assert( "Block id is 0" , b.block != 0 );

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
        delete &b;
        return f;
    }

    void FileSystem::gotoBlock( uint64_t blockId ) {
        l.enter( "GOTOBLOCK" );

//        Assert( "blockId is 0" , blockId , blockId > 0 );
        Assert( "Going out of range" , blockId, blocks_allocated, blockId < blocks_allocated );

        stream.seekp( blockId * Total_Size_Block );
        stream.seekg( blockId * Total_Size_Block );

        l.leave( "GOTOBLOCK" );
    }

    Block *FileSystem::readBlock( Block *ret ) {
        l.enter( "READBLOCK" );

        if( ret == NULL ) {
            ret = new Block();
        }
        ret->status = FULL;
        ret->block = stream.tellp() / Total_Size_Block;

        std::array<char,Total_Size_Block> blockBuffer;
        uint64_t *previous;
        uint64_t *next;
        uint64_t *length;
        char *data;

        previous    = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 0 * sizeof(uint64_t));
        next        = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 1 * sizeof(uint64_t));
        length      = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 2 * sizeof(uint64_t));
        data        = blockBuffer.begin()                               + 3 * sizeof(uint64_t);

        assertStream( stream );

        lock( READ );
        {
            stream.read( blockBuffer.data() , Total_Size_Block );
        }
        unlock( READ );

        ret->length = *length;
        ret->prev = *previous;
        ret->next = *next;
        std::copy( data , data + Block_Size , ret->data.begin() );

        assertStream( stream );
        l.leave( "READBLOCK" );

        return ret;
    }

    void FileSystem::writeBlock( Block &b ) {
        l.enter( "WRITEBLOCK" );


        std::array<char,Total_Size_Block> blockBuffer;
        uint64_t *previous;
        uint64_t *next;
        uint64_t *length;
        char *data;

        previous    = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 0 * sizeof(uint64_t));
        next        = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 1 * sizeof(uint64_t));
        length      = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 2 * sizeof(uint64_t));
        data        = blockBuffer.begin()                               + 3 * sizeof(uint64_t);

        uint64_t toWrite = sizeof(uint64_t) * 3;
        *previous = b.prev;
        *next = b.next;
        *length = b.length;
        if( b.status == FULL ) {
            toWrite = Total_Size_Block - (Block_Size - b.length);
            std::copy( b.data.begin() , b.data.begin() + b.length , data );
        }

        lock( WRITE );
        {
            gotoBlock( b.block );
            stream.write( blockBuffer.data() , toWrite );
            stream.flush();
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
            Assert( "Shoudl be at beginning" , metadata->position == 0 );
            Assert( "Should be at the beginning" , metaReader->file.position == 0 );

            for( int i = 0 ; i < metadata_files; ++i) {
                File &f = *(new File());

                uint64_t str_len,f_position = metaReader->tell(); 
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&str_len) );
                metaReader->read( str_len , buff.data() );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.disk_usage)) );
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.size)));
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.start)));
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.end)));
                metaReader->read( sizeof(uint64_t) , reinterpret_cast<char*>(&(f.metadata)));

                f.fs = this;
                f.name = std::string( buff.data() , str_len );
                f.current = f.start;

                allFilesMap[f.name] = &f;
            }
        }
        l.leave("LOADMETADATA");
    }

    void FileSystem::printHeader( bool force = false) {
        l.log( "Bytes Allocated", bytes_allocated , force );
        l.log( "Bytes Used", bytes_used , force );
        l.log( "Blocks Allocated", blocks_allocated , force );
        l.log( "Blocks Used", blocks_used , force );
        l.log( "Number free blocks", free_count , force );
        l.log( "First free block", free_first , force );
        l.log( "Bytes allocated for metadata", metadata_bytes_allocated , force );
        l.log( "Blocks allocated for metadata", metadata_allocated_blocks , force );
        l.log( "Bytes used for meta-data", metadata_bytes_used , force );
        l.log( "Blocks used for metadata", metadata_blocks_used , force );
        l.log( "Files created", metadata_files , force );
        l.log( "First block for metadata", metadata_start , force );
        l.log( "Last block for metadata", metadata_end , force );
    }

    void FileSystem::readHeader() {
        stream.seekg( SignatureSize , std::ios_base::beg );
        Assert( "Should have skipped signature", stream.tellg() == 8 );
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
        Assert( "Should have skipped signature", stream.tellp() == 8 );
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

    bool FileSystem::rename( File &toRename , const std::string &newName ) {
        l.enter( "RENAME" );
	const std::string fname = toRename.getFilename();
	if (openFilesMap.count(fname)) {
		File &file = *openFilesMap[fname];
		file.name = newName;
		toRename.name = newName;
		return true;
	}	
	return false;
/*
        for( auto file = openFiles.begin() ; file != openFiles.end() ; ++file ) {
            File &f = *(*file);
            if( f.getFilename() == toRename.getFilename() ) {
                f.name = newName;
                toRename.name = newName;
                return true;
            }
        }
*/
        l.leave( "RENAME" );
        return false;
    }


    void FileSystem::split( Block &b , uint64_t offset ) {
        l.enter( "SPLIT" );
        if( offset != b.length ) {

            Block &newBlock = *allocate( b.length - offset , b.data.begin() + offset );
            Block &next = *lazyLoad( b.next );

            newBlock.prev = b.block;
            newBlock.next = next.block;

            b.next = newBlock.block;
            next.prev = newBlock.block;

            b.length = offset;

            writeBlock( newBlock );
            writeBlock( b );
            writeBlock( next );

            delete &newBlock;
            delete &next;
        }
        l.leave( "SPLIT" );

    }


    void FileSystem::initBlocks(uint64_t start, uint64_t blocks, uint64_t buff_length , const char *buffer) {
        // Assertions
        Assert( "Why is it 1" , start > 1 );

        // Local variables
        std::array<char,Total_Size_Block> blockBuffer;
        uint64_t *previous;
        uint64_t *next;
        uint64_t *length;
        char *data;

        previous    = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 0 * sizeof(uint64_t));
        next        = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 1 * sizeof(uint64_t));
        length      = reinterpret_cast<uint64_t*>(blockBuffer.begin()   + 2 * sizeof(uint64_t));
        data        = blockBuffer.begin()                               + 3 * sizeof(uint64_t);

        // Save this for testing
        uint64_t initLength = buff_length;

        // Handle data
        if( buffer == NULL ) {
            initLength = 0;
            *length = 0;
            std::fill( data , data + Block_Size , 0 );
        }else {
            *length = Block_Size;
        }

        // Go to spot
        stream.seekp( start * Total_Size_Block );
        stream.seekg( start * Total_Size_Block );

        // Initialize previous and next
        *previous = start + blocks - 1;
        *next = start + 1;

        for( int i = 0 ; i < blocks - 1; ++i ) {

            if( buffer != NULL ) {
                std::copy( buffer , buffer + Block_Size , data );
                buff_length -= Block_Size;
            }

            // Write
            stream.write( blockBuffer.data() , Total_Size_Block );

            // Update
            *previous = *next - 1;
            *next = *next + 1;
        }

        // Final block
        *next = 0;
        if( buffer != NULL ) {
            *length = buff_length;
            std::copy( buffer , buffer + buff_length , data );
        }
        stream.write( blockBuffer.begin() , Total_Size_Block );
        stream.flush();

        // Validate
        /*
        for( int i = 0 ; i < blocks ; ++i ) {
            Block curr = lazyLoad( start + i );
            // Our length should be min of Block_Size and initLength
            Assert( "The length is not correct" , initLength , curr.length , curr.length == std::min( initLength , Block_Size) );
            initLength -= curr.length;

            if( i == 0 ) {
                Assert( "Previous does not point to end" , curr.prev == start + blocks - 1 );
            }else {
                Assert( "Previous does not point to previous" , curr.block , curr.prev , curr.prev == (start + i - 1) );
            }

            if( i == blocks - 1 ) {
                Assert( "Next is not 0" , curr.next == 0 );
            }else {
                Assert( "The next is 0" , curr.next != 0 );
            }
        }
        */
    }

    //  grow    -   Grow the filesystem enough to support writing the number of bytes given
    //
    //  bytes   -   How much space we need
    //
    //  @return -   The first block of the new space
    //
    Block *FileSystem::grow( uint64_t bytes , const char *buffer, Block* ret) {
        l.enter( "GROW" );

        // Handle clients data needs
        uint64_t blocks_to_write = (bytes + Block_Size - 1) / Block_Size;
        uint64_t current = blocks_allocated;

        // Handle free list
        free_first = blocks_allocated + blocks_to_write;
        free_count = Buffer_Size;

        // Grow base file
        lock( WRITE );
        {
            stream.seekp( Total_Size_Block * (blocks_allocated - 1) , std::ios_base::beg );
            stream.write( Zero , 1 );
            stream.flush();

            assertStream( stream );

            blocks_allocated += blocks_to_write + Buffer_Size;
            bytes_allocated = Total_Size_Block * blocks_allocated;
        }unlock( WRITE );

        initBlocks( free_first , free_count , 0 , NULL );
        initBlocks( current , blocks_to_write , bytes , buffer );

        l.leave( "GROW" );
        return load( current , ret );
    }


    //  load   -   Given a block id we load it from disk if not already loaded
    // 
    //  block       -   Block id we want to load
    //
    //  @return     -   Loaded block
    //
    Block *FileSystem::load( uint64_t block , Block *ret ) {
        l.enter( "LOAD" );
        assertStream(stream);
        //Assert( "Block should greater than 0" , block > 0 );

        gotoBlock( block );
        ret = readBlock(ret);

        l.leave( "LOAD" );

        return ret;
    }

    //  lazyLoad   - Given a block id only load meta-data
    //  
    //  block           - The block we wish to load
    //
    //  @return         - The loaded block
    //
    // TODO: Caching
    //
    Block *FileSystem::lazyLoad( uint64_t block , Block *ret  ) {
        l.enter( "LAZYLOAD" );

        if( ret == NULL ) {
            ret = new Block();
        }

        ret->block = block;
        ret->status = LAZY;

        std::array<char,3*sizeof(uint64_t)> buff;
        lock( READ );
        {
            gotoBlock( block );
            stream.read( buff.data() , 3 * sizeof(uint64_t) );
        }
        unlock( READ );

        ret->prev   = *reinterpret_cast<uint64_t*>(buff.begin() + 0 * sizeof(uint64_t) );
        ret->next   = *reinterpret_cast<uint64_t*>(buff.begin() + 1 * sizeof(uint64_t) );
        ret->length = *reinterpret_cast<uint64_t*>(buff.begin() + 2 * sizeof(uint64_t) );
        l.leave( "LAZYLOAD" );

        return ret;
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
    Block *FileSystem::reuse( uint64_t &length , const char* &buffer , Block* ret ) {
        l.enter( "REUSE" );

        assert( free_count > 0 );
        Assert( "Length is zero" , length > 0 );
        Assert( "Did I forget to change free_count or free_first" , free_first , free_count , free_first != 0 );

        uint64_t prevBlock = 0,currBlock = free_first;
        Block current;
        ret = lazyLoad( currBlock , ret );

        do {

            // Currently we treat the free_first as a special file, so just use it as such
            lazyLoad( currBlock , &current );
            current.status = FULL;
            current.length = std::min( length , Block_Size );
            uint64_t fillOffset = 0;
            if( buffer != NULL ) {
                std::copy( buffer , buffer + current.length , current.data.data() );
                fillOffset = Block_Size - current.length;
                buffer += current.length;
            }
            std::fill( current.data.begin() + fillOffset , current.data.begin() + Block_Size , 0 );

            // Update input
            
            length -= current.length;

            // Write to disk
            writeBlock( current );

            // Update free list
            free_first = current.next;
            --free_count;

            // Go to next
            prevBlock = currBlock;
            currBlock = free_first;
        } while ( free_count > 0 && length > 0 );

        current.next = 0;
        ret->prev = prevBlock;
        writeBlock( current );
        writeBlock( *ret );

        l.leave( "REUSE" );
        return ret;
    }

    //  allocate    -   Allocate enough bytes to support writing the data given
    //
    //  length      -   The amount of data in bytes
    //  buffer      -   The data
    //
    //  @return     -   The first block which holds our data
    //
    Block *FileSystem::allocate( uint64_t length , const char *buffer , Block* ret ) {
        l.enter( "ALLOCATE" );

        Assert( "The length cannot be zero" , length > 0 );

        assertStream( stream );

        if( free_count > 0 ){

            ret = reuse( length , buffer , ret );

            // If not enough room we have to grow
            if( length > 0 ) {
                Block grown,b_tmp;

                grow( length , buffer , &grown );

                // The very end
                uint64_t tmp = grown.prev;

                // Join in middle
                lazyLoad( ret->prev , &b_tmp );
                b_tmp.next = grown.block;
                grown.prev = ret->prev;

                // Point to actual end
                ret->prev = tmp;

                // Save
                writeBlock( b_tmp );
                writeBlock( *ret );
                writeBlock( grown );
            }
        }else {
            ret = grow( length , buffer , ret );
        }
        l.leave( "ALLOCATE" );
        return ret;
    }


    //  locate    -   Given a block id and an offset find the block which
    //                  has the data corresponding to that offset.
    //
    //  start       -   The starting block
    //  offset      -   How many bytes from the starting block, this is updated
    //
    //  @return     -   The block which holds the data pointed at by the offset
    //
    Block *FileSystem::locate( uint64_t start , uint64_t &offset , Block *ret ) {
        l.enter( "LOCATE" );

        if( ret == NULL ) {
            ret = new Block();
        }

        lazyLoad( start , ret );
        while( offset >= ret->length ) {
            offset -= ret->length;
            if( ret->next == 0 ) {
                break;
            }
            lazyLoad(ret->next, ret);
        }
        l.leave( "LOCATE" );
        return ret;
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

        assertStream( stream );

        uint64_t requested = length;
        length = std::min( length , file.size - file.position );

        if( length > 0 && buffer != 0 && file.position < file.size ) {
            Block current;
            load( file.current, &current );

            uint64_t next = current.block;
            load( next , &current );

            for( int i = 0 ; i < length ; ++i ) {
                buffer[i] = current.data[file.block_position];
                ++file.position;
                ++file.block_position;
                ++file.disk_position;
                if( file.block_position  == current.length ) {
                    if( current.next != 0 ) {
                        file.disk_position += Block_Size - current.length;
                        load( current.next, &current );
                        file.block_position = 0;
                        file.current = current.block;
                    }else {
                        break;
                    }
                }
            }
        }else {
            Assert( "If you have reached here something is really wrong" , false );
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

        // Grow internal file to be large enough
        if( length + file.disk_position > file.disk_usage ) {
            // Calculate how much extra space we need
            uint64_t growBy = round<Block_Size>(length - (file.disk_usage - file.disk_position));
            Block oldBlock;

            // Load for reconnection
            lazyLoad( file.end , &oldBlock );
            Block &newBlock = *allocate( growBy , NULL);

            // Update/Reconnect
            file.disk_usage += growBy;
            if( newBlock.prev == 0 ) {
                file.end = newBlock.block;
            }else {
                file.end = newBlock.prev;
            }
            oldBlock.next = newBlock.block;
            newBlock.prev = oldBlock.block;
            writeBlock( newBlock );
            writeBlock( oldBlock );

            // Save file data
            bytes_allocated += growBy;

            delete &newBlock;

        }

        // We assume we always have space to write to

        uint64_t curr = file.current;
        uint64_t written = 0;               // How much data we have written
        uint64_t overwritten = 0;           // How much data we have overwritten
        uint64_t to_overwrite = 0;          // How much left we need to remove

        if( file.position < file.size ) {
            to_overwrite = std::min( length , file.size - file.position );
        }


       // Assert( "The block id is 0" , curr != 0 );

        // While we have not written everything
        uint64_t remaining = length;
        while( written < length ) {

            // Go to the current block
            file.current = curr;
            Block b;
            load( curr , &b );

            // How much room is there to write our data
            uint64_t len = std::min( (Block_Size - file.block_position) , remaining );

            // How much data will we actually overwrite
            overwritten += (b.length + file.block_position);

            std::copy( buffer + written , buffer + written + len , b.data.data() + file.block_position );
            b.length = file.block_position + len;
            Assert( "We must make things full" , b.status == FULL );
            writeBlock( b );

            written += len;
            remaining -= len;

            file.position += len;
            file.block_position = 0;
            file.disk_position += len;

            curr = b.next;
        }

        file.size = to_overwrite;

        // Need to move blocks around
        if( overwritten < to_overwrite ) {
            //std::cout << "Overwritten: " << overwritten << " out of " << to_overwrite << std::endl;
            //Assert( false && "TODO: Handle case 'overwritten < length'"  );
        }else if( file.position > file.size  ) {
            bytes_used += file.position - file.size;
            file.size += file.position - file.size;
            bytes_used += file.position - file.size;;
        }

        // TODO: What happens if we fill up file exactly to end?

        if( &file != metadata ) { 
            insertFile( file );
        }

        l.leave( "WRITE" );
        return remaining;
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
        if( length > 0 ) {
            // Grow file if neccessary 
            if( file.position > file.size ) {
                Block oldEnd, remaining;

                // Grab the current end of file block
                lazyLoad( file.end , &oldEnd );

                // Actually allocate
                allocate( file.position - file.size , NULL , &remaining );

                // Update file
                file.end = remaining.prev;
                file.size += file.position - file.size;
                file.current = file.end;

                // Merge blocks together
                remaining.prev = oldEnd.block;
                oldEnd.next = remaining.block;

                writeBlock( remaining );
                writeBlock( oldEnd );

                delete &remaining;
            }
            Block b,next,end;

            locate( file.current , file.position , &b );

            // We might have to split
            split( b , file.position % Block_Size );

            // Now append to here
            allocate( length , buffer , &next );
            lazyLoad( next.prev , &end );
            end.next = b.next;
            next.prev = b.block;
            b.next = next.block;

            writeBlock( next );
            writeBlock( end );
            writeBlock( b );

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
        uint64_t remaining = std::min( length , file.size - file.position );

        // Make sure can remove bytes
        if( remaining > 0 && (file.size > file.position) ) {
            Block first,last;

            // Load first block we have to modify
            locate( file.start , file.position, &first);

            // Calculate how much to remove from first
            uint64_t removeFromFirst = std::min( remaining , first.length - file.position );
            // and how many bytes to skip to get to new spot
            remaining -= removeFromFirst;

            // Load final block
            locate( first.next , remaining , &last );
            load( last.block , &last );

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
            writeBlock( first );
            writeBlock( last );

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
            for( std::map<const std::string, File*>::iterator it = openFilesMap.begin() ; it != openFilesMap.end() ; ++it ) {
                File &file = *it->second;
                if( file.status == OPEN ) {
                    insertFile( file );
                    file.status = CLOSED;
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
        for( std::map<const std::string, File*>::iterator it = allFilesMap.begin() ; it != allFilesMap.end() ; ++it ) {
	    File *file = it->second;
            ret.push_back(file);
        }
        l.leave( "GETFILES" );
        return ret;
    }

    std::list<File*> FileSystem::getOpenFiles() {
        l.enter( "GETOPENFILES" );
        std::list<File*> ret;
        for( std::map<const std::string, File*>::iterator it = openFilesMap.begin() ; it != openFilesMap.end() ; ++it ) {
	    File *file = it->second;
            ret.push_back(file);
        }
        return ret;
        l.leave( "GETOPENFILES" );
    }

    std::list<std::string> FileSystem::getFilenames() {
        l.enter( "GETFILENAMES" );
        std::list<std::string> names;
        for( std::map<const std::string, File*>::iterator it = allFilesMap.begin() ; it != allFilesMap.end() ; ++it ) {
	    File *file = it->second;
            names.push_back( file->getFilename() );
        } 
        l.leave( "GETFILENAMES" );
        return names;
    }

    File& FileSystem::open( const std::string& name) {
        l.enter( "OPEN" );
	if (allFilesMap.count(name)) {
		File &file = *allFilesMap[name];
		openFilesMap[name] = &file;
		file.status = OPEN;
		l.leave("OPEN");
		return file;
	}
/*
        for( auto file = allFiles.begin() ; file != allFiles.end() ; ++file ) {
            if( (*file)->getFilename() == name ) {
                openFiles.push_back( *file );
                (*file)->status = OPEN;
                l.leave("OPEN");
                return **file;
            }
        } 
*/
        // Create new file
        File &f = createNewFile( name );
        f.status = OPEN;
	allFilesMap[name] = &f;
	openFilesMap[name] = &f;
//        allFiles.push_back( &f );
//        openFiles.push_back( &f );
        l.leave( "OPEN" );
        return f;
    }

    void FileSystem::close(const std::string& name) {
        l.enter( "CLOSE" );
	if (openFilesMap.count(name)) {
		File &file = *openFilesMap[name];
		if (file.status == OPEN) {
			file.status = CLOSED;
			file.position = 0;
			file.current = file.start;
			file.block_position = 0;
			file.disk_position = 0;
			openFilesMap.erase(name);
		}
	}
    }

    bool FileSystem::unlink( File &f ) {
        l.enter( "UNLINK:1" );
        l.leave( "UNLINK:1" );
        return false;
    }

    //
    bool FileSystem::unlink( const std::string& filename ) {
        l.enter( "UNLINK:2" );
        File& file = open(filename);
        l.leave( "UNLINK:2" );
        return file.unlink();
    }

    bool FileSystem::exists( const std::string& filename ) {
        l.enter( "EXISTS" );
        File& file = open(filename);
        l.leave( "EXISTS" );
        return file.getStatus() == OPEN;
    }


    FileSystem::FileSystem( const std::string& filename) {
        l.disable();
        l.enter( "FILESYSTEM" );

        fileSystemLocation = filename;

        //  Open the file
        stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

        if( !stream ) {
            stream.open( filename , std::fstream::binary | std::fstream::trunc | std::fstream::out );
            stream.close();
            stream.open( filename , std::fstream::in |  std::fstream::out | std::fstream::binary );

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
        for( std::map<const std::string, File*>::iterator it = openFilesMap.begin(); it != openFilesMap.end(); ++it ) {
            File &f = *it->second;
            if( f.getFilename() == fp.getFilename() ) {
                openFilesMap.erase(it);
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
