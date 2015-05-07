#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char *argv[])
{

	std::string fname("test.json");
	if (argc > 1) {
		fname = std::string(argv[1]);
	}

	std::ifstream in(fname);
	while (in.good()) {
		std::string line;
		std::getline(in, line);
		if (line.size() == 0) continue;
		std::stringstream data;
		try
		{
			data << line;
			boost::property_tree::ptree pt;
			boost::property_tree::read_json(data, pt);
			//write_json(std::cout,pt,true);
		}
		catch (std::exception const& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	in.close();

	return 0; 
}
