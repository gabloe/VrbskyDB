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

		bool insert() {
			std::string token = sc.nextToken();
			if (toLower(token).compare("into")) {
				std::cout << "PARSING ERROR: Expected 'into', found " << token << std::endl;
				return false;
			}
			std::string project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			std::string document = sc.nextToken();
			std::string json = sc.nextJSON();
			return true;
		}

		bool append() {
			std::string token = sc.nextToken();
			if (toLower(token).compare("to")) {
				std::cout << "PARSING ERROR: Expected 'to', found " << token << std::endl;
				return false;
			}
			std::string project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			std::string document = sc.nextToken();
			std::string json = sc.nextJSON();
			return true;
		}

		bool remove() {
			std::string token = sc.nextToken();
			if (toLower(token).compare("from")) {
				std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
				return false;
			}
			std::string project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			std::string document = sc.nextToken();
			token = sc.nextToken();
			if (toLower(token).compare("key")) {
				std::cout << "PARSING ERROR: Expected 'key', found " << token << std::endl;
				return false;
			}
			if (sc.nextChar() != '=') {
				std::cout << "PARSING ERROR: Expected an equals" << std::endl;
				return false;
			}
			std::string key = sc.nextString();
			return true;
		}

		bool select() {
			std::string token = sc.nextToken();
			if (toLower(token).compare("from")) {
				std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
				return false;
			}
			std::string project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			std::string document = sc.nextToken();
			if (wherePending()) {
				return where();
			}
			return true;
		}

		bool ddelete() {
			std::string token = sc.nextToken();
			if (!toLower(token).compare("document")) {
				std::string document = sc.nextToken();
				return true;
			}

			if (!toLower(token).compare("project")) {
				std::string project = sc.nextToken();
				return true;
			}
			return false;
		}

		bool create() {
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
				if (withValuePending()) {
					std::string json = sc.nextJSON();
				}
			}
			return true;

		}

		bool where() {
			std::string token = sc.nextToken();
			if (toLower(token).compare("key")) {
				std::cout << "PARSING ERROR: Expected 'key', found " << token << std::endl;
				return false;
			}	
			std::string value;
			if (sc.nextChar() == '=') {
				value = sc.nextString();
			} else {
				sc.push_back(1);
			}
			token = sc.nextToken();
			if (toLower(token).compare("in")) {
				std::cout << "PARSING ERROR: Expected 'in', found " << token << std::endl;
				return false;
			}
			value = sc.nextString();
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

		bool withValuePending() {
			std::string with = sc.nextToken();
			std::string value = sc.nextToken();
			bool found = false;
			if (!toLower(with).compare("with") && !toLower(value).compare("value")) {
				found = true;
			} else {
				sc.push_back(value);
				sc.push_back(with);
			}
			return found;
		}

		bool wherePending() {
			bool result = false;
			std::string token = sc.nextToken();
			if (!toLower(token).compare("where")) {
				result = true;
			} else {
				sc.push_back(token);
			}
			return result;
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
	Parsing::Parser p("create document Herp in Test with value { \"A\": 1, \"B\": 2 };");
	if (!p.parse()) {
		std::cout << "Parsing failure." << std::endl;
	} else {
		std::cout << "Parsing success." <<std::endl;
	}
	return 0;
}
