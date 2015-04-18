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
	File f = fs.load("Test");
	Storage::HashmapWriter<uint64_t> writer(f, fs);
	writer.write(data);

	std::map<std::string, uint64_t> res;
	Storage::HashmapReader<uint64_t> reader(f, fs);
	res = reader.read();
	std::cout << res["Hello"] << std::endl;
	std::cout << res["World"] << std::endl;

	std::string lorem("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suscipit vestibulum metus conubia duis mollis aliquet nisl elit ullamcorper.Egestas vehicula montes dictumst lacinia ultrices etiam.Fringilla vehicula aenean ipsum platea tellus per turpis taciti mauris tortor; nostra gravida.Nunc venenatis neque quam et magnis nam litora sem.Nunc ipsum dapibus auctor metus condimentum; netus lectus habitant erat tempus? Congue consequat lacinia cras elementum amet netus imperdiet?Egestas consequat Eros cubilia laoreet bibendum penatibus sed in: tempus sit sem?Faucibus platea nullam.Leo sodales Euismod enim dictum porta malesuada lorem.Commodo hac natoque leo praesent fames integre cursus accumsan lectus - magnis ultrices gravida. Mus est vehicula sagittis ligula aenean; quam cubilia natoque placerat sodales habitant: id eget fusce.Ridiculus eros volutpat maecenas vitae convallis ante turpis lectus facilisi habitant; etiam aliquam nam curabitur.Suscipit proin consequat et posuere eleifend facilisis accumsan torquent gravida.Neque nibh at morbi eu purus interdum sollicitudin mattis dis sociis; imperdiet taciti ut fusce.Donec ridiculus semper nisi mattis sociis curae lorem? Eros eu magnis sociis pretium aliquam.Egestas nunc vitae aliquam cum.Ad lacus lacinia amet et libero ullamcorper cum...Fringilla felis conubia sodales fermentum scelerisque magna, dis sapien senectus aliquam.Nec est dictumst dignissim platea lacinia vel amet pharetra - elit ullamcorper dolor aliquam in ut.");
	for (char i='a'; i<'z'; i++) {
		std::string name("");
		name += i;
		File f = fs.load(name);
		fs.write(&f, lorem.c_str(), lorem.size());
	}

	for (char i='a'; i<'z'; i++) {
		std::string name("");
		name += i;
		File f = fs.load(name);
		char *x = fs.read(&f);
		std::string out(x, f.size);
		std::cout << out << std::endl;
	}


	fs.shutdown();
}
