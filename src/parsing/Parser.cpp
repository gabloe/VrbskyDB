#include <iostream>
#include <string>
#include "Scanner.h"

namespace Parsing {
	enum Command {
		CREATE,
		INSERT,
		APPEND,
		REMOVE,
		SELECT,
		DELETE
	};
	enum Aggregate {
		NONE,
		AVG,
		SUM,
		STDEV
	};
	struct List {
		std::string value;
		List *next;
	};
	struct Query {
		Command command;
		Aggregate aggregate;
		std::string project;
		List *documents;
		std::string key;
		std::string value;
		void print() {
			std::cout << "Command: " << command << std::endl;
			std::cout << "Aggregate: " << aggregate << std::endl;
			std::cout << "Project: " << project << std::endl;
			List *spot = documents;
			while (spot != NULL) {
				std::cout << "Document: " << spot->value << std::endl;
				spot = spot->next;
			}
			std::cout << "Key: " << key << std::endl;
			std::cout << "Value: " << value << std::endl;
		}
	};
	class Parser {
	public:
		Parser(std::string query): sc(query) {}

		Query* parse() {
			std::string token = toLower(sc.nextToken());
			Query *q = new Query();
			bool result = false;
			if (!token.compare("create")) {
				result = create(*q);
			} else if (!token.compare("insert")) {
				result = insert(*q);
			} else if (!token.compare("append")) {
				result = append(*q);
			} else if (!token.compare("remove")) {
				result = remove(*q);
			} else if (!token.compare("select")) {
				result = select(*q);
			} else if (!token.compare("delete")) {
				result = ddelete(*q);
			}

			if (!result) {
				exit(1);
			}

			try {
				char t = sc.nextChar();
				if (t != ';') {
					std::cout << "PARSING ERROR: Expected semicolon but found '" << t << "'" << std::endl;
					exit(1);
				}	
			} catch (std::runtime_error &e) {
				std::cout << "PARSING ERROR: End of query reached.  Expected a semicolon." << std::endl;
				exit(1);
			}

			return q;
		}

		bool insert(Query &q) {
			q.command = INSERT;
			std::string token = sc.nextToken();
			if (toLower(token).compare("into")) {
				std::cout << "PARSING ERROR: Expected 'into', found " << token << std::endl;
				return false;
			}
			q.project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			q.documents = new List();
			q.documents->value = sc.nextToken();
			q.value = sc.nextJSON();
			return true;
		}

		bool append(Query &q) {
			q.command = APPEND;
			std::string token = sc.nextToken();
			if (toLower(token).compare("to")) {
				std::cout << "PARSING ERROR: Expected 'to', found " << token << std::endl;
				return false;
			}
			q.project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			q.documents = new List();
			q.documents->value = sc.nextToken();
			q.value = sc.nextJSON();
			return true;
		}

		bool remove(Query &q) {
			q.command = REMOVE;
			std::string token = sc.nextToken();
			if (toLower(token).compare("from")) {
				std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
				return false;
			}
			q.project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			q.documents = new List();
			q.documents->value = sc.nextToken();
			token = sc.nextToken();
			if (toLower(token).compare("where")) {
				std::cout << "PARSING ERROR: Expected 'where', found " << token << std::endl;
				return false;
			}
			token = sc.nextToken();
			if (toLower(token).compare("key")) {
				std::cout << "PARSING ERROR: Expected 'key', found " << token << std::endl;
				return false;
			}
			if (sc.nextChar() != '=') {
				std::cout << "PARSING ERROR: Expected an equals" << std::endl;
				return false;
			}
			q.key = sc.nextString();
			if (andValuePending()) {
				q.value = sc.nextJSON();
			}
			return true;
		}

		bool select(Query &q) {
			q.command = SELECT;
			std::string token = sc.nextToken();
			if (toLower(token).compare("from")) {
				std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
				return false;
			}
			q.project = sc.nextToken();
			if (sc.nextChar() != '.') {
				std::cout << "PARSING ERROR: Expected a dot" << std::endl;
				return false;
			}
			q.documents = new List();
			q.documents->value = sc.nextToken();
			if (wherePending()) {
				return where();
			}
			return true;
		}

		bool ddelete(Query &q) {
			q.command = DELETE;
			std::string token = sc.nextToken();
			if (!toLower(token).compare("document")) {
				q.project = sc.nextToken();
				if (sc.nextChar() != '.') {
					std::cout << "PARSING ERROR: Expected a dot" << std::endl;
					return false;
				}
				q.documents = new List();
				q.documents->value = sc.nextToken();
				return true;
			}

			if (!toLower(token).compare("project")) {
				q.project = sc.nextToken();
				return true;
			}
			return false;
		}

		bool create(Query &q) {
			q.command = CREATE;
			std::string token = toLower(sc.nextToken());
			if (!token.compare("project")) {
				q.project = sc.nextToken();
				if (withDocumentsPending()) {
					if (sc.nextChar() == '(') {
						q.documents = idList();
					} else {
						sc.push_back(1);
						throw std::runtime_error("PARSING ERROR: Expected open paren.");
					}
				}
			} else if (!token.compare("document")) {
				q.project = sc.nextToken();
				if (sc.nextChar() != '.') {
					std::cout << "PARSING ERROR: Expected a dot." << std::endl;
					return false;
				}
				q.documents = new List();
				q.documents->value = sc.nextToken();
				if (withValuePending()) {
					q.value = sc.nextJSON();
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

		List *idList() {
			List *doc = new List();
			std::string id = sc.nextToken();
			if (id.size() == 0) {
				throw std::runtime_error("PARSING ERROR: Expected identifier.");
			}
			doc->value = id;
 			char next = sc.nextChar();
			if (next == ',') {
				doc->next = idList();
			} else if (next == ')') {
				return doc;
			} else {
				throw std::runtime_error("PARSING ERROR: Expected closed paren.");
			}
			delete doc;
			return NULL;
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

		bool andValuePending() {
			bool result = false;
			std::string aand = sc.nextToken();
			std::string value = sc.nextToken();
			if (!toLower(aand).compare("and") && !toLower(value).compare("value") && sc.nextChar() == '=') {
				result = true;
			} else {
				sc.push_back(aand);
				sc.push_back(value);
				sc.push_back(1);
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
	Parsing::Parser p("create project test;");
	Parsing::Query *q = p.parse();
	if (!q) {
		std::cout << "Parsing failure." << std::endl;
	} else {
		std::cout << "Parsing success." <<std::endl;
		std::cout << "Query:" << std::endl;
		q->print();
	}
	return 0;
}
