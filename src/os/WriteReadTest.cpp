
#include <iostream>
#include <string>

#include "FileSystem.h"
#include "File.h"
#include "FileWriter.h"
#include "FileReader.h"

int main(void) {
    char data[] = {"Hello"};
    char test[] = {"     "};
    os::FileSystem fs( "database.db" );

    os::File &first = fs.open( "TEST" );
    os::FileWriter out( first );
    out.write( sizeof(data) , data );
    out.close();

    os::File& second = fs.open( "TEST" );
    os::FileReader in( second );
    in.read( sizeof(test) , test );
    in.close();

    std::cout << "Data Written: " << std::string( data , sizeof(data) ) << std::endl;
    std::cout << "Data Read: " << std::string( test , sizeof(test) ) << std::endl;

    fs.shutdown();
    return 0;
}
