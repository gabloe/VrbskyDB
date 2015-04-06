

#include "FileReader.h"
#include <string>

int main() {
    os::FileSystem fs( "test.dat" );
    os::File& f = fs.open( "TEST" );
    os::FileReader reader( f );
    std::string data = reader.readAll();
    std::cout << data << std::endl;
    return 0;
}
