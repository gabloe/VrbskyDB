#include <iostream>
#include <cassert>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

const size_t DataSize = 2 * 1024;

void writeData() {
    const char MyData[] = "Jello World";
    os::FileSystem fs( "test.data" );
    os::File &file = fs.open( "TEST" );
    os::FileWriter writer( file );
    writer.write( sizeof( MyData ) , MyData );
    assert( file.size == sizeof(MyData));
}

int main( void ) {
    writeData();
    return 0;
}
