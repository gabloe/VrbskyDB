#include <iostream>

#include "FileSystem.h"
#include "File.h"

#include "FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    fs.shutdown();

    return 0;
}
