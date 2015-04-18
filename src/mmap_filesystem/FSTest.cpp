#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	std::string test("ABCDEFGHIJKLMNOPQRSTUVWXYZ Hello World!  HERP DERP.  CS609 is the coolest.");
	for (char i='a'; i<'z'; ++i) {
		std::string fname("");
		fname += i;
		File f = fs.load(fname);
		fs.write(&f, test.c_str(), test.size());
	}

	for (char i='a'; i<'z'; ++i) {
		std::string fname("");
		fname += i;
		File f = fs.load(fname);
		char *data = fs.read(&f);
		std::string out(data, f.size);
		std::cout << out << std::endl;
	}
	fs.shutdown();
}
