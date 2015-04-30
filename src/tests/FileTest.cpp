#include <iostream>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"
#include "../assert/Assert.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &f = fs.open( "TEST" );
    Assert( "File is not empty" , f.size == 0 );
    fs.shutdown();

    return 0;
}
