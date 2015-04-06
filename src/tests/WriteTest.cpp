#include <iostream>

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
    writer.close();

    fs.shutdown();

    return 0;
}
