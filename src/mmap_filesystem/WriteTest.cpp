#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>
#include "HashmapWriter.h"
#include "HashmapReader.h"

void printFiles() {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
    auto files = fs->getFilenames();
    for( auto n = files.begin() ; n != files.end() ; ++n ) {
        std::cout << *n << std::endl;
        File f = fs->open_file( *n );
        std::cout << f.size << std::endl;
    }
	fs->shutdown();
    delete fs;
}

void write( std::string prefix ) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	std::string lorem("{\"A\": 1};");
	for (int i=0; i<10; i++) {
		std::string name( prefix + std::to_string(i));
		File f = fs->open_file(name);
		fs->write(&f, lorem.c_str(), lorem.size());
	}
	fs->shutdown();
    delete fs;
}

int main(void) {
    write( "herp" );
    printFiles();
    write( "derp" );
    printFiles();
    return 0;
}
