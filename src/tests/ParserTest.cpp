#include "../parsing/Parser.h"
#include <fstream>

int main(void) {
	std::ifstream infile("queries");
	std::string line;
	while (std::getline(infile, line)) {
		std::cout << "Query: " << line << std::endl;
		Parsing::Parser p(line);
		Parsing::Query *q = p.parse();
		if (q) {
			q->print();
		} else {
			std::cout << "ERROR!" << std::endl;
		}
		std::cout << std::endl;
        std::cout << "--------------------------------------------------------------" << std::endl;
	}	
	return 0;
}
