#include "Filesystem.h"
#include <iostream>
#include <string>
#include <map>
#include "HashmapWriter.h"
#include "HashmapReader.h"

int main(void) {
	Storage::Filesystem fs("data.db");
	std::map<std::string, uint64_t> data;
	data["Hello"] = 1;
	data["World"] = 2;
	File f = fs.open_file("Test");
	Storage::HashmapWriter<uint64_t> writer(f, fs);
	writer.write(data);

	std::map<std::string, uint64_t> res;
	Storage::HashmapReader<uint64_t> reader(f, fs);
	res = reader.read();
	std::cout << res["Hello"] << std::endl;
	std::cout << res["World"] << std::endl;

	std::string lorem("insert into test with {\"A\": 1};");
	for (int i=0; i<5000; i++) {
		std::string name(std::to_string(i));
		File f = fs.open_file(name);
		fs.write(&f, lorem.c_str(), lorem.size());
	}

	for (int i=0; i<5000; i++) {
		std::string name(std::to_string(i));
		File f = fs.open_file(name);
		char *x = fs.read(&f);
		std::string out(x, f.size);
		std::cout << out << std::endl;
		free(x);
	}

	fs.shutdown();
}
