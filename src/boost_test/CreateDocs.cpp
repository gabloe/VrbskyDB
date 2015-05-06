#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{
    int numDocs = 100;
    int numFields = 1;
    if (argc > 1) {
	numDocs = atoi(argv[1]);
    }
    if (argc > 2) {
        numFields = atoi(argv[2]);
    }

    std::string sep;
    for (int i = 0 ; i < numDocs ; ++i) {
	std::stringstream data;
	data << "{\"" << std::to_string(i) << "\" : {";
	sep = ",";
	for (int j = 0 ; j < numFields ; ++j) {
		if (j == numFields - 1) {
			sep = "";
		}
		data << "\"" << std::to_string(i) << std::to_string(j) << "\" : " << std::to_string(j) << sep;
	}
	data << "}}";
	std::cout << data.str() << std::endl;
    }
    return 0; 
}
