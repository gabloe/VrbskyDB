#include <iostream>

#include "../mmap_filesystem/Filesystem.h"
#include "../assert/Assert.h"

int main( void ) {
    const std::string FileName = "test.dat";
    Storage::Filesystem fs( FileName );
    File f = fs.open_file( "TEST" );
    Assert( "File is not empty" , f.size == 0 );
    fs.shutdown();

    return 0;
}
