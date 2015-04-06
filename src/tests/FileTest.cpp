#include <iostream>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &f = fs.open( "TEST" );
    fs.shutdown();

    return 0;
}
