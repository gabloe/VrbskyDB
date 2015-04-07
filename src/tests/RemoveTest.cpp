#include <iostream>
#include <cassert>

#include "../os/FileSystem.h"
#include "../os/File.h"

#include "../os/FileWriter.h"

int main( void ) {
    const char MyData[] = "Jello World";
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &file = fs.open( "TEST" );
    os::FileWriter writer( file );
    writer.write( sizeof( MyData ) , MyData );
    assert( file.size == sizeof( MyData ) );
    writer.close();

    fs.shutdown();

    return 0;
}
