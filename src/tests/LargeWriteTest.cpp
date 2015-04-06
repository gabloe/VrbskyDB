#include <iostream>

#include "FileSystem.h"
#include "File.h"

#include "FileWriter.h"

int main( void ) {
    const int Size = 100 * 1024;
    char *Data = new char[Size];
    char c = 'a';
    for( int i = 0 ; i < Size; ++i ) {
        Data[i] = c;
        ++c;
        if( c > 'z' ) c = 'a';
    }
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File &file = fs.open( "TEST" );
    os::FileWriter writer( file );
    writer.write( Size , Data );
    writer.close();
    fs.shutdown();

    delete Data;

    return 0;
}
