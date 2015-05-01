#include <iostream>

#include "../mmap_filesystem/Filesystem.h"

int main( void ) {
    const std::string FileName = "test.dat";
    Storage::Filesystem fs( FileName );
    fs.shutdown();

    return 0;
}
