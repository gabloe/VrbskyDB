#include <iostream>
#include <string>
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"

int main(int argc, char **argv) {
	std::string q = "";
	while (1) {
		std::cout << "Enter a query (q to quit):" << std::endl;
		getline(std::cin, q);
		if (!q.compare("q")) {
			break;
		}
		Parsing::Parser p(q);
		Parsing::Query *query = p.parse();
		if (query) {
			query->print();
		}
		std::cout << std::endl;
	}
	return 0;
}
