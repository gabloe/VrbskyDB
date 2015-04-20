#include "Filesystem.h"

int main(void) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
    auto files = fs->getFilenames();
    for( auto n = files.begin() ; n != files.end() ; ++n ) {
        std::cout << *n << std::endl;
        File f = fs->open_file( *n );
        std::cout << f.size << std::endl;
    }
	fs->shutdown();
    delete fs;
    return 0;
}
