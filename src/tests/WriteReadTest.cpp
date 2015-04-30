
#include <iostream>
#include <string>
#include <cassert>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"
#include "../os/FileReader.h"

int main(void) {
    char data[] = {"Hello"};
    char test[] = {"     "};
    os::FileSystem fs( "test.dat" );

    os::File &first = fs.open( "TEST" );
    os::FileWriter out( first );
    out.write( sizeof(data) , data );
    assert(first.size == sizeof(data));
    out.close();

    os::File& second = fs.open( "TEST" );
    assert(second.size == sizeof(data));
    os::FileReader in( second );
    in.read( sizeof(test) , test );
    in.close();

    for( size_t i = 0 ; i < sizeof(data) ; ++i ) assert( data[i] == test[i] );

    fs.shutdown();
    return 0;
}
