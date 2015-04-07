

#include <string>

void Assert( std::string msg , bool test ) {
    if( test == false ) {
        std::cout << msg << std::endl;
        std::exit( -1 );
    }
}

template<class K,class L>
void Assert( std::string msg , K val1 , L val2 , bool test ) {
    if( test == false ) {
        std::cout << msg << ": " << val1 << ", " << val2 << std::endl;
        std::exit( -1 );
    }
}

template<class K>
void Assert( std::string msg , K val , bool test ) {
    if( test == false ) {
        std::cout << msg << ": " << val << std::endl;
        std::exit( -1 );
    }
}
