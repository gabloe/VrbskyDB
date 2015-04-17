#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	File file1 = fs.load("test1");
	fs.write(&file1, "Hello, World!!!", sizeof("Hello, World!!!"));

	File file2 = fs.load("test2");
	fs.write(&file2, "Herp Derp", sizeof("Herp Derp"));

	char *data1 = fs.read(&file1);
	char *data2 = fs.read(&file2);

	std::string first(data1, file1.size);
	std::string second(data2, file2.size);
	
	std::cout << first << std::endl;
	std::cout << second << std::endl;

	fs.shutdown();
}
