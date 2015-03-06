/*
Query parser for the NoSQL database management system.
*/

#include <iostream>
#include <string>
#include "Scanner.h"

namespace Parsing {
	const std::string Aggregates[] = {"AVG", "SUM", "STDEV" /*, TODO: Others. */};
	enum Command {
		CREATE,
		INSERT,
		APPEND,
		REMOVE,
		SELECT,
		DELETE
	};
	enum Aggregate {
		NONE = -1,
		AVG = 0,
		SUM = 1,
		STDEV = 2
	};
	struct List {
		std::string value;
		List *next;
		List(std::string value_): value(value_), next(NULL) {}
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
			q->aggregate = NONE;
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
				delete q;
				return NULL;
			}

			try {
				char t = sc.nextChar();
				if (t != ';') {
					std::cout << "PARSING ERROR: Expected semicolon but found '" << t << "'" << std::endl;
					delete q;
					return NULL;
				}	
			} catch (std::runtime_error &e) {
				std::cout << "PARSING ERROR: End of query reached.  Expected a semicolon." << std::endl;
				delete q;
				return NULL;
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
			q.documents = new List(sc.nextToken());
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
			q.documents = new List(sc.nextToken());
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
			q.documents = new List(sc.nextToken());
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
			if (aggregatePending()) {
				aggregate(q);
			}
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
			q.documents = new List(sc.nextToken());
			if (wherePending()) {
				return where(q);
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
				q.documents = new List(sc.nextToken());
				return true;
			} else if (!toLower(token).compare("project")) {
				q.project = sc.nextToken();
				return true;
			} else {
				std::cout << "PARSING ERROR: Expected 'document' or 'project'." << std::endl;
				return false;
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
						try {
							q.documents = idList();
						} catch (std::runtime_error &e) {
							std::cout << e.what() << std::endl;
							return false;
						}
					} else {
					        std::cout << "PARSING ERROR: Expected open paren." << std::endl;
						return false;
					}
				}
			} else if (!token.compare("document")) {
				q.project = sc.nextToken();
				if (sc.nextChar() != '.') {
					std::cout << "PARSING ERROR: Expected a dot." << std::endl;
					return false;
				}
				q.documents = new List(sc.nextToken());
				if (withValuePending()) {
					q.value = sc.nextJSON();
				}
			} else {
				std::cout << "PARSING ERROR: Expected 'document' or 'project'." << std::endl;
				return false;
			}
			return true;

		}

		bool where(Query &q) {
			std::string token = sc.nextToken();
			if (toLower(token).compare("key")) {
				std::cout << "PARSING ERROR: Expected 'key', found " << token << std::endl;
				return false;
			}	
			if (sc.nextChar() == '=') {
				q.key = sc.nextString();
				return true;
			} else {
				sc.push_back(1);
			}
			token = sc.nextToken();
			if (toLower(token).compare("in")) {
				std::cout << "PARSING ERROR: Expected 'in', found " << token << std::endl;
				return false;
			}
			q.key = sc.nextString();
			return true;
		}

		void aggregate(Query &q) {
			std::string token = sc.nextToken();
			int numAggregates = sizeof(Aggregates) / sizeof(std::string);
			for (int i=0; i<numAggregates; ++i) {
				if (!toLower(token).compare(toLower(Aggregates[i]))) {
					q.aggregate = (Aggregate)i;
				}
			}
		}

		List *idList() {
			std::string id = sc.nextToken();
			if (id.size() == 0) {
				std::cout << "PARSING ERROR: Expected identifier." << std::endl;
				return NULL;
			}
			List *doc = new List(id);
			char next = sc.nextChar();
			if (next == ',') {
				doc->next = idList();
			} else if (next == ')') {
				return doc;
			} else {
				throw std::runtime_error("PARSING ERROR: Expected closed paren.");
			}
			return doc;
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

		bool aggregatePending() {
			std::string token = sc.nextToken();
			bool result = false;
			int numAggregates = sizeof(Aggregates) / sizeof(std::string);
			for (int i=0; i<numAggregates; ++i) {
				if (!toLower(token).compare(toLower(Aggregates[i]))) {
					result = true;
				}
			}
			sc.push_back(token);
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
	Parsing::Parser p("select avg from *.* where key=\"ABC\";");
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
