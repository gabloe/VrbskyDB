#include <iostream>
#include <string>
#include <algorithm>

#include "../assert/Assert.h"

#include "Filesystem.h"
#include "HashmapWriter.h"
#include "HashmapReader.h"

void validate(std::string prefix , int howMany) {
    bool *tests = new bool[howMany];
    for( int i = 0 ; i < howMany; ++i) tests[i] = false;

	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
    auto files = fs->getFilenames();
    for( auto n = files.begin() ; n != files.end() ; ++n ) {
        File f = fs->open_file( *n );
        Assert( "File size is incorrect" , f.size, f.size == 9 );
        Assert( "Wrong prefix" , f.name.find( prefix) == 0 );
        int idx = atoi( f.name.data() + prefix.size() );
        Assert( "Number is wrong", idx , idx < howMany && idx >= 0 );
        Assert( "Already seen this number", tests[idx] == false );
        tests[idx] = true;
    }
	fs->shutdown();

    for( int i = 0 ; i < howMany; ++i) Assert( "Missing a file", i  , tests[i] );

    delete fs;
    delete tests;

}

void printFiles(int limit) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
    auto files = fs->getFilenames();
    for( auto n = files.begin() ; limit-- && n != files.end() ; ++n ) {
        File f = fs->open_file( *n );
        std::cout << *n << ": " << f.size << std::endl;
    }
	fs->shutdown();
    delete fs;
}

void write( std::string prefix , int limit ) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	std::string lorem("{\"A\": 1};");
	for (int i=0; i < limit; i++) {
		std::string name( prefix + std::to_string(i));
		File f = fs->open_file(name);
		fs->write(&f, lorem.c_str(), lorem.size());
	}
	fs->shutdown();
    delete fs;
}

int main(void) {
    write( "herp" , 100 );
    validate("herp" , 100 );
    printFiles(10);
    return 0;
}
