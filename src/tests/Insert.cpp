
#include <exception>
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc , char *argv[]) {
    int generate_count;

    if( argc != 2 ) {
        std::cout << "Usage: " << argv[0] << " <num_documents>" << std::endl;
        return -1;
    }

    try {
        generate_count = atoi( argv[1] );
    }catch (std::exception &e)  {
        std::cout << argv[1] << " is not a valid number" << std::endl;
        return -1;
    }
    for( int i = 0 ; i < generate_count; ++i ) {
        std::cout << "insert into herp with { \"A\" : " << i << " };" << std::endl;
    }
    std::cout << "q" << std::endl;
    return 0;
}

