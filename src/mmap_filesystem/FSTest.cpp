#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	File file1 = fs.load("test1");

	std::cout << "Writing some short text to the file." << std::endl;
	fs.write(&file1, "Hello, World!!!", sizeof("Hello, World!!!"));

	char *data1 = fs.read(&file1);

	std::string first(data1, file1.size);
	
	std::cout << first << std::endl;

	std::cout << "Overwriting the text with something long." << std::endl;
	std::string data("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	fs.write(&file1, data.c_str(), data.size());

	char *data2 = fs.read(&file1);
	std::string derp(data2, file1.size);
	std::cout << derp << std::endl;

	fs.shutdown();
}
