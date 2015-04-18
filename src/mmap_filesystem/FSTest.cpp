#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	std::string test("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Non dictumst pulvinar.Ligula quis Malesuada nibh luctus quam morbi tellus - nec est amet sodales augue iaculis nisi.Blandit ullamcorper rutrum habitasse urna netus ridiculus diam.Semper Aptent luctus senectus a habitasse consectetur etiam sollicitudin arcu augue: iaculis massa.Facilisi ipsum nam faucibus senectus porttitor lorem nisl: risus id enim elementum parturient. Suspendisse elit potenti placerat nulla mauris nam, ullamcorper egestas lectus tellus nisl pharetra?Elit vehicula purus habitant ullamcorper velit imperdiet egestas lorem: netus arcu id sodales pulvinar...Commodo euismod maecenas accumsan per aenean convallis - fusce iaculis cursus felis at ultricies?Dis vehicula velit vestibulum aliquet molestie.Ligula maecenas libero etiam cursus. Metus viverra Mattis vestibulum sodales elementum ad.Commodo ligula nascetur facilisi nunc auctor: mauris nam pretium viverra nisl id.Scelerisque feugiat facilisi interdum viverra fames penatibus lobortis: pulvinar iaculis.Phasellus feugiat Magnis quisque nec est dolor.Semper magnis porttitor pharetra pulvinar enim ultricies cras! Nibh consequat nec netus neque.Feugiat nunc luctus posuere porta congue...Ligula fringilla sit purus pellentesque vulputate magnis porttitor risus sollicitudin erat.Scelerisque himenaeos facilisi sit hac gravida - libero ac congue pulvinar lacinia bibendum.Nunc vitae metus libero convallis est.");
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
