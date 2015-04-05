#include <cassert>
#include <string>

#include "FileReader.h"

int main() {
    os::FileSystem fs( "test.dat" );
    os::File& f = fs.open( "TEST" );
    os::FileReader reader( f );
    char buff[1024];
    reader.read( 1024 , buff );
    char c = 'a';
    for( int i = 0 ; i < 1024; ++i ) {
        assert( c == buff[i] );
        ++c;
        if( c > 'z' ) c = 'a';
    }
    std::cout << std::string( buff , 1024 ) << std::endl;
    return 0;
}
