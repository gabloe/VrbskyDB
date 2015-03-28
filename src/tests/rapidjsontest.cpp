#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iostream>

int main(void) {
	rapidjson::Document *d = new rapidjson::Document();
	d->SetArray();
	rapidjson::Value v;
	v.SetString("Hello");
	d->PushBack(v, d->GetAllocator());


	std::string ser = toString(d);
	std::cout << ser << std::endl;
	return 0;
}
