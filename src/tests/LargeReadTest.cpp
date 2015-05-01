#include <cassert>
#include <string>

#include "../mmap_filesystem/FileReader.h"
#include "../mmap_filesystem/FileWriter.h"

const size_t DataSize = 2 * 1024;

void writeData() {
    Storage::FileSystem fs( "test.dat" );
    File& file = fs.open_file( "TEST" );

    char *data = new char[DataSize];
    char c = 'a';
    for( size_t i = 0 ; i < DataSize ; ++i ) {
        data[i] = c;
        ++c;
        if( c > 'z' ) {
            c = 'a';
        }
    }
    fs.write( &file , data , DataSize );
    assert( file.size == DataSize );
    delete[] data;
    fs.shutdown();
}

void readData() {
    Storage::FileSystem fs( "test.dat" );
    File& file = fs.open_file( "TEST" );

    assert(file.size == DataSize);

    char *data = fs.read( &file );

    char c = 'a';
    for( size_t i = 0 ; i < DataSize ; ++i ) {
        assert( data[i] == c );
        ++c;
        if( c > 'z' ) {
            c = 'a';
        }
    }
    fs.shutdown();
}


int main() {
    writeData();
    readData();
    return 0;
}
