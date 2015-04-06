#include <iostream>

#include "FileSystem.h"
#include "File.h"

#include "FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &f = fs.open( "TEST" );
    fs.shutdown();

    return 0;
}
