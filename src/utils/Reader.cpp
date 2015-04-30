
#include <iostream>
#include <fstream>
#include <cstdint>
#include <inttypes.h>
#include <cstring>
#include <array>
#include <cassert>
#include <string>


#define moveAndTest(stream,pos ) {  \
    pos += 8;                       \
    assert( (uint64_t)stream.tellg() == pos );\
}

const uint64_t blockSize = 1024;
static const std::array<char,8> sign = { {0xD , 0xE , 0xA , 0xD , 0xB , 0xE , 0xE , 0xF}  };

struct File {
    std::string name;
    uint64_t str_len;
    uint64_t disk;
    uint64_t size;
    uint64_t start;
    uint64_t end;
    uint64_t metadata;
};

struct FileSystem {
    uint64_t bytes_allocated;
    uint64_t bytes_used;
    uint64_t blocks_allocated;
    uint64_t blocks_used;
    uint64_t free_count;
    uint64_t free_first;
    uint64_t metadata_bytes_allocated;
    uint64_t metadata_allocated_blocks;
    uint64_t metadata_bytes_used;
    uint64_t metadata_blocks_used;
    uint64_t metadata_files;
    uint64_t metadata_start;
    uint64_t metadata_end;
};

void printFile( File &f ) {
    std::cout << "File:" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "\tFilename: " << f.name << std::endl;
    std::cout << "\tDisk Usage: " << f.disk << std::endl;
    std::cout << "\tSize: " << f.size << std::endl;
    std::cout << "\tStart: " << f.start << std::endl;
    std::cout << "\tEnd: " << f.end << std::endl;
    std::cout << "\tMetadata: " << f.metadata << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
}

struct FileStatus {
    uint64_t block,length,next,prev,pos;
    std::array<char,1000> buffer;
};

void readBlock( std::fstream &stream , uint64_t block , FileStatus &status ) {
    assert( block != 0 );
    status.pos = 0;
    status.block = block;
    stream.seekg( block * 1024 );
    stream.read( reinterpret_cast<char*>(&(status.prev)),sizeof(uint64_t));
    stream.read( reinterpret_cast<char*>(&(status.next)),sizeof(uint64_t));
    stream.read( reinterpret_cast<char*>(&(status.length)),sizeof(uint64_t));
    stream.read( status.buffer.data() , 1000 );

    std::cout << "Block" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "\tCurrent Block: " << status.block << std::endl;
    std::cout << "\tPrevious Block: " << status.prev << std::endl;
    std::cout << "\tNext block: " << status.next << std::endl;
    std::cout << "\tLength: " << status.length << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
}

uint64_t read(std::fstream &stream , FileStatus &status , char* buffer , size_t length  ) {
    uint64_t pos = 0;
    while( pos < length ) {
        uint64_t toRead = std::min( status.length - status.pos , length - pos );
        std::copy( status.buffer.data() + status.pos , status.buffer.data() + status.pos + toRead , buffer + pos );
        pos += toRead;
        status.pos += toRead;

        if( status.pos == status.length ) {
            if( status.next == 0 ) break;
            readBlock( stream , status.next , status );
        }
    }
    return length - pos;
}

void printFiles( std::fstream &stream, FileSystem &fs ) {
    uint64_t block = fs.metadata_start;
    FileStatus status;
    readBlock( stream , block , status  );

    std::array<char,1024> filename;

    for( int idx = 0 ; idx < fs.metadata_files ; ++idx ) {
        File f;
        read( stream , status , reinterpret_cast<char*>(&(f.str_len)) , sizeof(uint64_t) );
        read( stream , status , filename.data() , f.str_len );
        read( stream , status , reinterpret_cast<char*>(&(f.disk)) , sizeof(uint64_t) );
        read( stream , status , reinterpret_cast<char*>(&(f.size)) , sizeof(uint64_t) );
        read( stream , status , reinterpret_cast<char*>(&(f.start)) , sizeof(uint64_t) );
        read( stream , status , reinterpret_cast<char*>(&(f.end)) , sizeof(uint64_t) );
        read( stream , status , reinterpret_cast<char*>(&(f.metadata)) , sizeof(uint64_t) );
        f.name = std::string( filename.data() , f.str_len );

        printFile( f );
    }
}

void printFileSystem( FileSystem &fs ) {
    std::cout << "Bytes allocated on disk: " << fs.bytes_allocated << std::endl;
    std::cout << "Bytes used: " << fs.bytes_used << std::endl;
    std::cout << "Allocated blocks: " << fs.blocks_allocated << std::endl;
    std::cout << "Blocks used: " << fs.blocks_used << std::endl;
    std::cout << "Free blocks: " << fs.free_count << std::endl;
    std::cout << "First Free block: " << fs.free_first << std::endl;
    std::cout << "Bytes allocated to metadata: " << fs.metadata_bytes_allocated << std::endl;
    std::cout << "Blocks allocated to metadata: " << fs.metadata_allocated_blocks << std::endl;
    std::cout << "Bytes used by metadata: " << fs.metadata_bytes_used << std::endl;
    std::cout << "Blocks ued by metadata: " << fs.metadata_blocks_used << std::endl;
    std::cout << "Number of files: " << fs.metadata_files << std::endl;
    std::cout << "First block of metadata: " << fs.metadata_start << std::endl;
    std::cout << "Last block of metadata: " << fs.metadata_end << std::endl;
}

void read64( std::fstream &stream , uint64_t &t ) {
    stream.read( reinterpret_cast<char*>( &t ) , sizeof(uint64_t) );
}


int main( int argc, char *argv[] ) {

    std::cout << "Size is " << sizeof(uint64_t) << std::endl;

    std::fstream stream( argv[1] , std::fstream::binary | std::fstream::in );

    std::cout << "Opening file " << argv[1] << " for reading" << std::endl;

    if( !stream || !stream.good()) {
        std::cout << "Error opening file" << std::endl;
        return -1;
    }

    std::array<char,8> buff;

    FileSystem fs;
    int pos = 0;
    stream.read( buff.data() , 8 );
    if( std::strncmp( buff.data() , sign.data() ,8) != 0 ) {
        std::cout << "herp" << std::endl;
        std::exit( -1 );
    }
    moveAndTest(stream,pos);
    read64( stream , fs.bytes_allocated );
    moveAndTest(stream,pos);
    read64( stream , fs.bytes_used );
    read64( stream , fs.blocks_allocated );
    read64( stream , fs.blocks_used );
    read64( stream , fs.free_count );
    read64( stream , fs.free_first );
    read64( stream , fs.metadata_bytes_allocated );
    read64( stream , fs.metadata_allocated_blocks );
    read64( stream , fs.metadata_bytes_used );
    read64( stream , fs.metadata_blocks_used );
    read64( stream , fs.metadata_files );
    read64( stream , fs.metadata_start );
    read64( stream , fs.metadata_end );

    printFileSystem( fs );

    printFiles( stream , fs );

    return 0;
}
