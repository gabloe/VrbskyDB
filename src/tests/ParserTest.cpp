#include "../Parsing/parser.h"
#include <fstream>

int main(void) {
	std::ifstream infile("queries");
	std::string line;
	while (std::getline(infile, line)) {
		Parsing::Parser p(line);
		Parsing::Query *q = p.parse();
		std::cout << "Query: " << line << std::endl;
		if (q) {
			q->print();
		} else {
			std::cout << "ERROR!" << std::endl;
		}
		std::cout << std::endl;
	}	
	return 0;
}
