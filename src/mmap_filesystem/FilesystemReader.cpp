#include <iostream>
#include "Filesystem.h"
#include <string>
#include <string.h>
#include <vector>

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Need filename." << std::endl;
		exit(1);
	}

	std::string fname(argv[1], strlen(argv[1]));
	Storage::Filesystem *fs = new Storage::Filesystem(fname);
	std::cout << "Files:" << std::endl;
	std::vector<std::string> fnames = fs->getFilenames();
	for (auto it = fnames.begin(); it != fnames.end(); ++it) {
		std::cout << *it << ":" << std::endl;
		File f = fs->open_file(*it);
		std::cout << "\tStarts at block: " << f.block << std::endl;
		std::cout << "\tFile size: " << f.size << std::endl;
		char *data = fs->read(&f);
		std::string out(data, f.size);
		std::cout << "\tData: " << out << std::endl;
	}
		
	fs->shutdown();
}
