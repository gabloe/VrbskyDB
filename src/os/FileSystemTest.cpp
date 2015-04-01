#include <iostream>

#include "FileSystem.h"
#include "File.h"

#include "FileWriter.h"

int main( void ) {
    const char MyData[] = "Hello World";
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File file = fs.open( "Test" );
    os::FileWriter writer( file );
    writer.write( sizeof( MyData ) , MyData );
    writer.close();

    fs.shutdown();

    return 0;
}
