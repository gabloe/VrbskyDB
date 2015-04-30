
#include <iostream>
#include <cstdint>
#include <string>

#include "HerpHash.h"
#include "../assert/Assert.h"

int main(int argc, char *argv[]) {

    uint64_t HowMany = 500;

    if( argc == 2 ) {
        HowMany = atoi( argv[1] );
    }

    HowMany = std::max( HowMany, (uint64_t)0 );

    Storage::HerpHash<std::string,uint64_t> lol;
    for( uint64_t i = 0 ; i < HowMany; ++i ) {
        std::string key = std::to_string(i);
        uint64_t value = i;
        lol.put( key , value );
    }

    // Displaying everything
    for( auto iter = lol.begin() ; iter != lol.end() ; ++iter ) {
        std::pair<std::string,int> pair = **iter;
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    return 0;
}
