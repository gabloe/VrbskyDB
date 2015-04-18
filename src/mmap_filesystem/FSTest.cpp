#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	File file = fs.load("test");
	std::cout << "Writing file with something short." << std::endl;
	fs.write(&file, "Hello, World!", 13);
	char *y = fs.read(&file);
	std::string out2(y, file.size);
	std::cout << "Result should be 'Hello, World!':" << std::endl << out2 << std::endl;
	free(y);
	fs.shutdown();
}
