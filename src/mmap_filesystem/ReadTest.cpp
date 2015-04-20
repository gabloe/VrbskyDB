#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>

#include "HashmapWriter.h"
#include "HashmapReader.h"

void prefixRead(std::string prefix, int howMany) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
    for (int i=0; i< howMany; i++) {
        std::string name(prefix + std::to_string(i));
        File f = fs->open_file(name);
        if( f.size > 0 ) {
            std::cout << "Filesize: " << f.size << std::endl;
            char *x = fs->read(&f);
            if  (x != NULL) {
                std::string out(x, f.size);
                std::cout << out << std::endl;
                free(x);
            }
        }
    }
    fs->shutdown();
    delete fs;
}

int main() {
    prefixRead( "herp" , 10 );
    prefixRead( "derp" , 10 );
    return 0;
}
