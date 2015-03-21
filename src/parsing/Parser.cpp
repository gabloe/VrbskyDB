/*
Query parser for the NoSQL database management system.
*/

#include <iostream>
#include <string>
#include <algorithm>
#include "Scanner.h"
#include "Parser.h"

std::string toLower(std::string s) {
	std::string ss = std::string(s);
	std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);
	return ss;
}

Parsing::Query* Parsing::Parser::parse() {
	std::string token = toLower(Parsing::Parser::sc.nextToken());
	Parsing::Query *q = new Parsing::Query();
	bool result = false;
	if (!token.compare("create")) {
		result = create(*q);
	} else if (!token.compare("insert")) {
		result = insert(*q);
	} else if (!token.compare("alter")) {
		result = alter(*q);
	} else if (!token.compare("remove")) {
		result = remove(*q);
	} else if (!token.compare("select")) {
		result = select(*q);
	} else if (!token.compare("delete")) {
		result = ddelete(*q);
	} else if (!token.compare("show")) {
		result = show(*q);
	} else {
		std::cout << "PARSING ERROR: Expected a valid command, but found '" << token << "'" << std::endl;
	}

	if (!result) {
		delete q;
		return NULL;
	}

	try {
		char t = Parsing::Parser::sc.nextChar();
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

bool Parsing::Parser::show(Parsing::Query &q) {
	q.command = SHOW;
	std::string token = Parsing::Parser::sc.nextToken();
	if (!toLower(token).compare("projects")) {
		q.project = new std::string("__PROJECTS__");
	} else if (!toLower(token).compare("documents")) {
		std::string in = Parsing::Parser::sc.nextToken();
		if (toLower(in).compare("in")) {
			std::cout << "PARSING ERROR: Expected 'in'." << std::endl;
			Parsing::Parser::sc.push_back(in);
			Parsing::Parser::sc.push_back(token);
			return false;
		}
		q.project = new std::string(Parsing::Parser::sc.nextToken());
	} else {
		std::cout << "PARSING ERROR: Expected 'projects' or 'documents'" << std::endl;
		return false;
	}
	return true;
}

bool Parsing::Parser::insert(Parsing::Query &q) {
	q.command = INSERT;
	std::string token = Parsing::Parser::sc.nextToken();
	if (toLower(token).compare("into")) {
		std::cout << "PARSING ERROR: Expected 'into', found " << token << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	if (Parsing::Parser::sc.nextChar() != '.') {
		std::cout << "PARSING ERROR: Expected a dot" << std::endl;
		return false;
	}
	q.documents = new List(Parsing::Parser::sc.nextToken());
	q.value = new std::string(Parsing::Parser::sc.nextJSON());
	return true;
}

bool Parsing::Parser::alter(Parsing::Query &q) {
	q.command = ALTER;
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	if (Parsing::Parser::sc.nextChar() != '.') {
		Parsing::Parser::sc.push_back(1);
		std::cout << "PARSING ERROR: Expected a dot" << std::endl;
		return false;
	}
	q.documents = new List(Parsing::Parser::sc.nextToken());
	std::string token = Parsing::Parser::sc.nextToken();
	if (!toLower(token).compare("add")) {
		q.value = new std::string(Parsing::Parser::sc.nextJSON());
	} else {
		Parsing::Parser::sc.push_back(token);
		std::cout << "PARSING ERROR: Expected 'add', found " << token << "." << std::endl;
		return false;
	}
	return true;
}

bool Parsing::Parser::remove(Parsing::Query &q) {
	q.command = REMOVE;
	try {
		q.keys = keyList();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return false;
	}
	std::string token = Parsing::Parser::sc.nextToken();
	if (toLower(token).compare("from")) {
		Parsing::Parser::sc.push_back(token);
		std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	if (Parsing::Parser::sc.nextChar() != '.') {
		Parsing::Parser::sc.push_back(1);
		std::cout << "PARSING ERROR: Expected a dot" << std::endl;
		return false;
	}
	q.documents = new List(Parsing::Parser::sc.nextToken());
	return true;
}

bool Parsing::Parser::select(Parsing::Query &q) {
	q.command = SELECT;
	try {
		q.keys = keyList();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return false;
	}
	std::string token = Parsing::Parser::sc.nextToken();
	if (toLower(token).compare("from")) {
		Parsing::Parser::sc.push_back(token);
		std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	if (Parsing::Parser::sc.nextChar() != '.') {
		Parsing::Parser::sc.push_back(1);
		std::cout << "PARSING ERROR: Expected a dot" << std::endl;
		return false;
	}
	q.documents = new List(Parsing::Parser::sc.nextToken());
	return true;
}

bool Parsing::Parser::ddelete(Parsing::Query &q) {
	q.command = DELETE;
	std::string token = Parsing::Parser::sc.nextToken();
	if (!toLower(token).compare("document")) {
		q.project = new std::string(Parsing::Parser::sc.nextToken());
		if (Parsing::Parser::sc.nextChar() != '.') {
			std::cout << "PARSING ERROR: Expected a dot" << std::endl;
			return false;
		}
		q.documents = new List(Parsing::Parser::sc.nextToken());
		return true;
	} else if (!toLower(token).compare("project")) {
		q.project = new std::string(Parsing::Parser::sc.nextToken());
		return true;
	} else {
		std::cout << "PARSING ERROR: Expected 'document' or 'project'." << std::endl;
		return false;
	}

	return false;
}

bool Parsing::Parser::create(Parsing::Query &q) {
	q.command = CREATE;
	std::string token = toLower(Parsing::Parser::sc.nextToken());
	if (!token.compare("project")) {
		q.project = new std::string(Parsing::Parser::sc.nextToken());
		if (withDocumentsPending()) {
			if (Parsing::Parser::sc.nextChar() == '(') {
				try {
					q.documents = idList();
				} catch (std::runtime_error &e) {
					Parsing::Parser::sc.push_back(1);
					std::cout << e.what() << std::endl;
					return false;
				}
			} else {
				Parsing::Parser::sc.push_back(1);
				std::cout << "PARSING ERROR: Expected open paren." << std::endl;
				return false;
			}
		}
	} else if (!token.compare("document")) {
		q.project = new std::string(Parsing::Parser::sc.nextToken());
		if (Parsing::Parser::sc.nextChar() != '.') {
			Parsing::Parser::sc.push_back(1);
			std::cout << "PARSING ERROR: Expected a dot." << std::endl;
			return false;
		}
		q.documents = new List(Parsing::Parser::sc.nextToken());
		if (withValuePending()) {
			q.value = new std::string(Parsing::Parser::sc.nextJSON());
		}
	} else {
		Parsing::Parser::sc.push_back(token);
		std::cout << "PARSING ERROR: Expected 'document' or 'project'." << std::endl;
		return false;
	}
	return true;

}

Parsing::List *Parsing::Parser::keyList() {
	Parsing::List *item = new Parsing::List();
	if (aggregatePending()) {
		if (!aggregate(item)) {
			throw std::runtime_error("PARSING ERROR: Malformed aggregate.");
		}
	} else {
		std::string id = Parsing::Parser::sc.nextToken();
		if (id.size() == 0) {
			Parsing::Parser::sc.push_back(id);
			throw std::runtime_error("PARSING ERROR: Expected an identifier.");
		} else if (id.at(0) == '_') {
			Parsing::Parser::sc.push_back(id);
			throw std::runtime_error("PARSING ERROR: Key cannot begin with an underscore.");
		}
		item->value = id;
	}
	char next = Parsing::Parser::sc.nextChar();
	if (next == ',') {
		item->next = keyList();
	} else {
		Parsing::Parser::sc.push_back(1);
	}
	return item;

}

bool Parsing::Parser::aggregate(Parsing::List *list) {
	std::string token = Parsing::Parser::sc.nextToken();
	if (Parsing::Parser::sc.nextChar() != '(') {
		std::cout << "PARSING ERROR: Expected open parenthesis." << std::endl;
		return false;
	}
	list->value = Parsing::Parser::sc.nextToken();
	if (Parsing::Parser::sc.nextChar() != ')') {
		std::cout << "PARSING ERROR: Expected closed parenthesis." << std::endl;
		return false;
	}
	int numAggregates = sizeof(Aggregates) / sizeof(std::string);
	for (int i=0; i<numAggregates; ++i) {
		if (!toLower(token).compare(toLower(Aggregates[i]))) {
			list->aggregate = new Aggregate((Aggregate)i);
		}
	}
	return true;
}

Parsing::List * Parsing::Parser::idList() {
	std::string id = Parsing::Parser::sc.nextToken();
	if (id.size() == 0) {
		Parsing::Parser::sc.push_back(id);
		throw std::runtime_error("PARSING ERROR: Expected an identifier.");
	} else if (id.at(0) == '_') {
		Parsing::Parser::sc.push_back(id);
		throw std::runtime_error("PARSING ERROR: Key cannot begin with an underscore.");
	}
	List *doc = new List(id);
	char next = Parsing::Parser::sc.nextChar();
	if (next == ',') {
		doc->next = idList();
	} else if (next == ')') {
		return doc;
	} else {
		Parsing::Parser::sc.push_back(1);
		throw std::runtime_error("PARSING ERROR: Expected closed paren.");
	}
	return doc;
}

bool Parsing::Parser::withDocumentsPending() {
	std::string with = Parsing::Parser::sc.nextToken();
	std::string documents = Parsing::Parser::sc.nextToken();
	bool found = false;
	if (!toLower(with).compare("with") && !toLower(documents).compare("documents")) {
		found = true;
	} else {
		Parsing::Parser::sc.push_back(documents);
		Parsing::Parser::sc.push_back(with);
	}
	return found;
}

bool Parsing::Parser::withValuePending() {
	std::string with = Parsing::Parser::sc.nextToken();
	std::string value = Parsing::Parser::sc.nextToken();
	bool found = false;
	if (!toLower(with).compare("with") && !toLower(value).compare("value")) {
		found = true;
	} else {
		Parsing::Parser::sc.push_back(value);	
		Parsing::Parser::sc.push_back(with);	
	}
	return found;
}

bool Parsing::Parser::aggregatePending() {
	std::string token = Parsing::Parser::sc.nextToken();
	bool result = false;
	int numAggregates = sizeof(Aggregates) / sizeof(std::string);
	for (int i=0; i<numAggregates; ++i) {
		if (!toLower(token).compare(toLower(Aggregates[i]))) {
			result = true;
		}
	}
	Parsing::Parser::sc.push_back(token);
	return result;
}
