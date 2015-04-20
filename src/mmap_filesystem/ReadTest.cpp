#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>

#include "HashmapWriter.h"
#include "HashmapReader.h"

int main(void) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	for (int i=0; i<20; i++) {
		std::string name(std::to_string(i));
		File f = fs->open_file(name);
        if( f.size > 0 ) {
            std::cout << "Filesize: " << f.size << std::endl;
            char *x = fs->read(&f);
	    if  (x == NULL) {
		continue;
	    }
            std::string out(x, f.size);
            std::cout << out << std::endl;
            free(x);
        }
	}
	fs->shutdown();
    delete fs;
}
