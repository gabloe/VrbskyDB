
#include "FileSystem.h"
#include "File.h"


int main( void ) {
    const char MyData[] = "Hello World";
    const std::string FileName = "test.dat";
    os::FileSystem fs( FileName );
    os::File file = fs.open( "Test" );
    file.write( sizeof( MyData ) , MyData );
    file.close();
    fs.shutdown();
    
    return 0;
}
