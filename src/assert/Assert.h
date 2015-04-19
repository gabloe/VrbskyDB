
#ifndef ASSERT_H_
#define ASSERT_H_

#include <string>
#include <cstdlib>

template<class K,class L>
static inline void Assert( std::string msg , K val1 , L val2 , bool test ) {
    if( test == false ) {
        std::cout << msg << ": " << val1 << ", " << val2 << std::endl;
        std::cout.flush();
        abort();
    }
}

template<class K>
static inline void Assert( std::string msg , K val , bool test ) {
    if( test == false ) {
        std::cout << msg << ": " << val << std::endl;
        std::cout.flush();
        abort();
    }
}

static inline void Assert( std::string msg , bool test ) {
    if( test == false ) {
        std::cout << msg << std::endl;
        std::cout.flush();
        abort();
    }
}
#endif
