#include <cassert>
#include <string>

#include "../os/FileReader.h"
#include "../os/FileWriter.h"

const size_t DataSize = 2 * 1024;

void writeData() {
    os::FileSystem fs( "test.dat" );
    os::File& file = fs.open( "TEST" );
    os::FileWriter writer( file );

    char *data = new char[DataSize];
    char c = 'a';
    for( size_t i = 0 ; i < DataSize ; ++i ) {
        data[i] = c;
        ++c;
        if( c > 'z' ) {
            c = 'a';
        }
    }
    writer.write( DataSize , data );
    delete[] data;
    assert( file.size == DataSize );
    writer.close();
    fs.shutdown();
}

void readData() {
    os::FileSystem fs( "test.dat" );
    os::File& file = fs.open( "TEST" );
    os::FileReader reader( file );

    assert(file.size == DataSize);

    char *data = new char[DataSize];
    assert(reader.read( DataSize , data ) == 0);

    char c = 'a';
    for( size_t i = 0 ; i < DataSize ; ++i ) {
        assert( data[i] == c );
        ++c;
        if( c > 'z' ) {
            c = 'a';
        }
    }
    delete[] data;
    reader.close();
    fs.shutdown();
}

void singleByteReadData() {
    os::FileSystem fs( "test.dat" );
    os::File& file = fs.open( "TEST" );
    os::FileReader reader( file );

    assert(file.size == DataSize);

    char c = 'a';
    for( size_t i = 0 ; i < DataSize ; ++i ) {
        char t = 0;
        reader.read( 1 , &t );
        assert( t == c );
        ++c;
        if( c > 'z' ) {
            c = 'a';
        }
    }
    reader.close();
    fs.shutdown();
}


int main() {
    writeData();
    readData();
    singleByteReadData();
    return 0;
}
