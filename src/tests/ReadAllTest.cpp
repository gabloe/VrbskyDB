#include <string>

#include "../os/FileReader.h"
#include "../os/FileWriter.h"

const size_t DataSize = 2 * 1024;

char *genData() {
    char *data = new char[DataSize];
    for( size_t i = 0 ; i < DataSize; ++i ) data[i] = 'a' + (i % 'z');
    return data;
}

void check( char *data) {
    for( size_t i = 0 ; i < DataSize; ++i ) {
        char c = 'a' + i % 'z';
        assert( data[i] == c );
    }
}

void writeData() {
    os::FileSystem fs( "test.data" );
    os::File& file = fs.open( "TEST" );
    os::FileWriter writer( file );
    char *data = genData();
    writer.write(DataSize,data);
    assert(file.size == DataSize);
    delete[] data;
}

void readAllTest()  {
    os::FileSystem fs( "test.data" );
    os::File& file = fs.open( "TEST" );
    assert(file.size == DataSize);
    os::FileReader reader( file );
    char *data = reader.readAll();
    check(data);
    delete[] data;
}

int main() {
    writeData();
    readAllTest();
    return 0;
}
