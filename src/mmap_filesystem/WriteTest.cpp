#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>
#include "HashmapWriter.h"
#include "HashmapReader.h"

int main(void) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	std::string lorem("{\"A\": 1};");
	for (int i=0; i<10; i++) {
		std::string name(std::to_string(i));
		File f = fs->open_file(name);
		fs->write(&f, lorem.c_str(), lorem.size());
	}
	fs->shutdown();
    delete fs;
}
