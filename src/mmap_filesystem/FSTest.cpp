#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	File file = fs.load("test");
	std::string data("Hello, World!");
	fs.write(&file, data.c_str(), data.size());

	char *x = fs.read(&file);
	std::string out1(x, file.size);

	std::cout << out1 << std::endl;

	std::string data2("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	fs.write(&file, data2.c_str(), data2.size());

	char *y = fs.read(&file);
	std::string out2(y, file.size);

	std::cout << out2 << std::endl;
	
	free(x);
	free(y);

	fs.shutdown();
}
