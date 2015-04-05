

#include "FileReader.h"
#include <string>

int main() {
    os::FileSystem fs( "test.dat" );
    os::File& f = fs.open( "TEST" );
    os::FileReader reader( f );
    char buff[1024];
    reader.read( 12 , buff );
    buff[12] = 0;
    std::cout << std::string( buff , 12 ) << std::endl;
    return 0;
}
