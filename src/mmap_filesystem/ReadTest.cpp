#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>

#include "../assert/Assert.h"
#include "HashmapWriter.h"
#include "HashmapReader.h"

int main(void) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	for (int i=0; i<10; i++) {
		std::string name(std::to_string(i));
		File f = fs->open_file(name);
		char *x = fs->read(&f);
		std::string out(x, f.size);
		std::cout << out << std::endl;
		free(x);
	}
	fs->shutdown();
}
