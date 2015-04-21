
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

    bool *check = new bool[HowMany];

    Storage::HerpHash<std::string,uint64_t> lol;
    for( uint64_t i = 0 ; i < HowMany; ++i ) {
        std::string key = std::to_string(i);
        uint64_t value = i;
        lol.put( key , value );
    }

    uint64_t seen = 0;

    for( auto iter = lol.begin() ; iter != lol.end() ; ++iter ) {
        std::pair<const std::string,uint64_t>& kvp = *iter;
        Assert( "Already saw" , !check[kvp.second] );
        ++seen;
        check[kvp.second] = true;
    }
    Assert( "Missing some" , seen == HowMany );
    return 0;
}
