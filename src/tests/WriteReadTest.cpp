
#include <iostream>
#include <string>
#include <cassert>

#include "../mmap_filesystem/Filesystem.h"

int main(void) {
    char data[] = {"Hello"};
    char *test;
    Storage::Filesystem fs( "test.dat" );

    File first = fs.open_file( "TEST" );
    fs.write( &first , data , sizeof(data) );
    assert(first.size == sizeof(data));

    File second = fs.open_file( "TEST" );
    assert(second.size == sizeof(data));

    test = fs.read( &second );

    for( size_t i = 0 ; i < sizeof(data) ; ++i ) assert( data[i] == test[i] );
    free(test);

    fs.shutdown();
    return 0;
}
