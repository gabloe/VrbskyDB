
#include <fstream>
#include <cstdint>
#include <inttypes.h>

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
} FileSystem;

const uint64_t blockSize = 1024;

void printFileSystem( FileSystem &fs ) {
    printf( "%" PRIu64 "\n" , bytes_allocated);
    printf( "%" PRIu64 "\n" , bytes_used);
    printf( "%" PRIu64 "\n" , blocks_allocated);
    printf( "%" PRIu64 "\n" , blocks_used);
    printf( "%" PRIu64 "\n" , free_count);
    printf( "%" PRIu64 "\n" , free_first);
    printf( "%" PRIu64 "\n" , metadata_bytes_allocated);
    printf( "%" PRIu64 "\n" , metadata_allocated_blocks);
    printf( "%" PRIu64 "\n" , metadata_bytes_used);
    printf( "%" PRIu64 "\n" , metadata_blocks_used);
    printf( "%" PRIu64 "\n" , metadata_files);
    printf( "%" PRIu64 "\n" , metadata_start);
    printf( "%" PRIu64 "\n" , metadata_end);
}

uint64_t read64( std::fstream stream , &t ) {
        str.write( reinterpret_cast<char*>( &t ) , sizeof(t) );
}


int main( int argc, char *argv[] ) {
    std::fstream stream( argv[1] , std::fstream::binary | std::fstream::in );
    if( !stream ) {
        return -1;
    }

    FileSystem fs;

    read64( stream , fs.bytes_allocated );
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

    return 0;
}
