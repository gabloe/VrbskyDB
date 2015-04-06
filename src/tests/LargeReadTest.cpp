#include <cassert>
#include <string>

#include "FileReader.h"

int main() {
    os::FileSystem fs( "test.dat" );
    os::File& f = fs.open( "TEST" );
    std::cout << "File size: " << f.size << std::endl;
    os::FileReader reader( f );
    char buff[f.size];
    reader.read( f.size , buff );
    std::cout << std::string( buff , f.size ) << std::endl;
    return 0;
}
