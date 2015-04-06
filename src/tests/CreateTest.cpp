#include <iostream>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    fs.shutdown();

    return 0;
}
