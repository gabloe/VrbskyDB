#include "../storage/LinearHash.h"
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

void execute(Parsing::Query &, Storage::LinearHash<std::string> &);

void execute(Parsing::Query &q, Storage::LinearHash<std::string> &table) {
	q.print();
	switch (q.command) {
	case Parsing::CREATE:
		// Create the project and/or document with or without any values.
		{
			rapidjson::Document d;
			if(!d.Parse(q.value.c_str()).HasParseError()) {
				rapidjson::StringBuffer buffer;
    				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    				d.Accept(writer);
				std::cout << "\nResult from RapidJSON:" << std::endl;
				std::cout << buffer.GetString() << std::endl;
			} else {
				std::cout << "Could not parse JSON." << std::endl;
			}
		}
		break;
	case Parsing::SELECT:
		// Build a JSON object with the results
		break;
	case Parsing::DELETE:
		// Delete a project or document
		break;
	case Parsing::REMOVE:
		// Remove a key
		break;
	case Parsing::APPEND:
		// Append to a field
		break;
	default:
		std::cout << "Command not recognized." << std::endl;
	}
}

int main(void) {
	std::string q = "";
	Storage::LinearHash<std::string> table(1024, 2048);
	while (1) {
		std::cout << "Enter a query (q to quit):" << std::endl;
		getline(std::cin, q);
		if (!q.compare("q")) {
			break;
		}
		Parsing::Parser p(q);
		Parsing::Query *query = p.parse();
		if (query) {
			execute(*query, table);
		}
		std::cout << std::endl;
	}
	return 0;
}
