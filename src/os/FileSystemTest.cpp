#include <iostream>

#include "FileSystem.h"
#include "File.h"

#include "FileWriter.h"

int main( void ) {
    const char MyData[] = "Hello World";
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File file = fs.open( "Test" );
    os::FileWriter writer( file );
    writer.write( sizeof( MyData ) , MyData );
    writer.close();

    fs.shutdown();

    int num = 1;
    if(*(char *)&num == 1) {
       printf("\nLittle-Endian\n");
    } else {
       printf("Big-Endian\n");
    }
    
    return 0;
}
