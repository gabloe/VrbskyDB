
#ifndef OS_CONSTANTS_H_
#define OS_CONSTANTS_H_

#include <cstdint>

#include <iostream>

static uint64_t depth = 0;
static bool allEnabled = false;


template<uint64_t to>
uint64_t roundTo( uint64_t input ) {
    return to * ((input + to - 1)/to);
}

void tabs() {
    for( int i = 0 ; i < depth ; ++i ) std::cout << "\t";
}

template <class V>
void log( V msg , bool req = false ) {
    if( req || allEnabled ) {
        tabs();
        std::cout << msg << std::endl;                      
    }
}

template <class K,class V>
void log( K key , V value , bool req = false ) {
    if( req || allEnabled ) {
        tabs();
        std::cout << key << ": " << value << std::endl;
    }
}

#define enter(MSG) {    \
    log(MSG,true);      \
    ++depth;            \
}

#define leave(MSG) {    \
    --depth;            \
    log(MSG,true);      \
}


namespace os {


    enum FilePosition { BEG , CUR , END };
    enum FileStatus { OPEN , CLOSED , DELETED };
    enum LockType { READ , WRITE };

    static const uint64_t KB = 1024;
    static const uint64_t MB = 1024 * KB;
    static const uint64_t GB = 1024 * MB;
    static const uint64_t TB = 1024 * GB;

    template<uint64_t V>
        uint64_t round( uint64_t t ) {
            return V * ( (t + V - 1) / V);
        }

}

#endif
