#include <pretty.h>
#include <iostream>

void foo(rapidjson::Document *d) {
	std::string ser = toString(d);
	std::cout << ser << std::endl;
}

int main(void) {
	rapidjson::Document *d = new rapidjson::Document();
	d->SetArray();
	rapidjson::Value v;
	v.SetString("Hello");
	d->PushBack(v, d->GetAllocator());

	foo(d);
	return 0;
}
