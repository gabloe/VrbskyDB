#include <iostream>
#include <cassert>

#include "../os/FileSystem.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

#include "../assert/Assert.h"

int main( void ) {
    const size_t Size = 2 * 1024;
    char *Data = new char[Size];
    char c = 'a';
    for( size_t i = 0 ; i < Size; ++i ) {
        Data[i] = c;
        ++c;
        if( c > 'z' ) c = 'a';
    }

    os::FileSystem fs( "test.dat" );
    os::File &file = fs.open( "TEST" );
    os::FileWriter writer( file );

    writer.write( Size , Data );

    Assert( "Test for equality failed" , file.size , Size , file.size == Size);

    writer.close();
    fs.shutdown();

    delete Data;

    return 0;
}
