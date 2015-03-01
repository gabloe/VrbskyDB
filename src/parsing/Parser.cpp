#include <iostream>
#include <string>
#include "Scanner.h"

namespace Parsing {
	class Parser {
	public:
		Parser(std::string query): sc(query) {}

		bool parse() {
			std::string token = toLower(sc.nextToken());

			bool result = false;
			if (!token.compare("create")) {
				result = create();
			} else if (!token.compare("insert")) {
				result = insert();
			} else if (!token.compare("append")) {
				result = append();
			} else if (!token.compare("remove")) {
				result = remove();
			} else if (!token.compare("select")) {
				result = select();
			} else if (!token.compare("delete")) {
				result = ddelete();
			}

			try {
				char t = sc.nextChar();
				if (t != ';') {
					std::cout << "PARSING ERROR: Expected semicolon but found '" << t << "'" << std::endl;
					return false;
				}	
			} catch (std::runtime_error &e) {
				std::cout << "PARSING ERROR: End of query reached.  Expected a semicolon." << std::endl;
				return false;
			}

			return result;
		}

		// TODO: These functions...
		bool insert() {
			return false;
		}

		bool append() {
			return false;
		}

		bool remove() {
			return false;
		}

		bool select() {
			return false;
		}

		bool ddelete() {
			return false;
		}

		bool create() {
			return creations();
		}

		bool creations() {
			std::string token = toLower(sc.nextToken());
			if (!token.compare("project")) {
				std::string projectName = sc.nextToken();
				if (withDocumentsPending()) {
					if (sc.nextChar() == '(') {
						return idList();
					} else {
						sc.push_back(1);
						throw std::runtime_error("PARSING ERROR: Expected open paren.");
					}
				}
			} else if (!token.compare("document")) {
				std::string documentName = sc.nextToken();
				if (!sc.nextToken().compare("in")) {
					std::string projectName = sc.nextToken();
				} else {
					throw std::runtime_error("PARSING ERROR: Expected 'in'.");
				}
			}
			return true;
		}

		bool idList() {
			bool result = false;
			std::string id = sc.nextToken();
			if (id.size() == 0) {
				throw std::runtime_error("PARSING ERROR: Expected identifier.");
			}
			std::cout << id << std::endl;
 			char next = sc.nextChar();
			if (next == ',') {
				result = idList();
			} else if (next == ')') {
				result = true;
			} else {
				throw std::runtime_error("PARSING ERROR: Expected closed paren.");
			}
			return result;
		}

		bool withDocumentsPending() {
			std::string with = sc.nextToken();
			std::string documents = sc.nextToken();
			bool found = false;
			if (!toLower(with).compare("with") && !toLower(documents).compare("documents")) {
				found = true;
			} else {
				sc.push_back(documents);
				sc.push_back(with);
			}
			return found;
		}

	private:
		Scanner sc;
		std::string toLower(std::string s) {
			std::string ss = std::string(s);
			std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);
			return ss;
		}
	};
}

int main(int argc, char **argv) {
	Parsing::Parser p("create document derp in test;");
	if (!p.parse()) {
		std::cout << "Parsing failure." << std::endl;
	} else {
		std::cout << "Parsing success." <<std::endl;
	}
	return 0;
}
