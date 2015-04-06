#include <iostream>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &f = fs.open( "TEST" );
    std::cout << "File opened and has size " << f.size << std::endl;
    fs.shutdown();

    return 0;
}
