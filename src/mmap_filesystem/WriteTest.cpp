#include <iostream>
#include <string>
#include <algorithm>

#include "../assert/Assert.h"

#include "Filesystem.h"
#include "HashmapWriter.h"
#include "HashmapReader.h"

const std::string db_name = "data.db";

void validate(std::string prefix , int howMany) {
    bool *tests = new bool[howMany];
    for( int i = 0 ; i < howMany; ++i) tests[i] = false;

	Storage::Filesystem *fs = new Storage::Filesystem( db_name );
    auto files = fs->getFilenames();
    int count = 0;
    for( auto n = files.begin() ; n != files.end() ; ++n ) {
        File f = fs->open_file( *n );
        Assert( "File size is incorrect" , f.size, f.size == 9 );
        Assert( "Wrong prefix" , f.name.find( prefix) == 0 );
        int idx = atoi( f.name.data() + prefix.size() );
        Assert( "Number is wrong", idx , idx < howMany && idx >= 0 );
        Assert( "Already seen this number", tests[idx] == false );
        tests[idx] = true;
        ++count;
    }

    if( count != howMany ) {
        std::cout << "Missing " << howMany - count << " file(s) out of  " << howMany << " files" << std::endl;
    }
	fs->shutdown();

    for( int i = 0 ; i < howMany; ++i) if( !tests[i] ) {std::cerr << "Missing file " << i << std::endl;};

    delete fs;
    delete tests;

}

void printFiles(int limit) {
	Storage::Filesystem *fs = new Storage::Filesystem( db_name );
    auto files = fs->getFilenames();
    for( auto n = files.begin() ; limit-- && n != files.end() ; ++n ) {
        File f = fs->open_file( *n );
        std::cout << *n << ": " << f.size << std::endl;
    }
	fs->shutdown();
    delete fs;
}

void write( std::string prefix , int limit ) {
	Storage::Filesystem *fs = new Storage::Filesystem( db_name );
	std::string lorem("{\"A\": 1};");
	for (int i=0; i < limit; i++) {
		std::string name( prefix + std::to_string(i));
		File f = fs->open_file(name);
		fs->write(&f, lorem.c_str(), lorem.size());
	}
	fs->shutdown();
    delete fs;
}

int main( int argc , char *argv[]) {
    int limit = 5000;
    if( argc == 2 ) {
        limit = atoi(argv[1] );
    }
    write( "herp" , limit );
    validate("herp" , limit );
    return 0;
}
