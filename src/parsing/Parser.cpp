/*
Query parser for the NoSQL database management system.
*/

#include <iostream>
#include <string>
#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <pretty.h>
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
	} else if (!token.compare("update")) {
		result = update(*q);
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

bool Parsing::Parser::create(Parsing::Query &q) {
	q.command = CREATE;
	std::string index = toLower(Parsing::Parser::sc.nextToken());
	std::string on = toLower(Parsing::Parser::sc.nextToken());
	if (!index.compare("index") && !on.compare("on")) {
		q.fields = new rapidjson::Document();
		char c = Parsing::Parser::sc.nextChar();
		std::string arr; 
		if (c == '[') {
			Parsing::Parser::sc.push_back(1);
			arr = Parsing::Parser::sc.nextJSON();
		} else {
			arr += '[';
			Parsing::Parser::sc.push_back(1);
			std::string field = Parsing::Parser::sc.nextToken();
			if (field.find("\"") != 0) {
				arr += '"';
			}
			arr += field; 
			if (field.find("\"") != field.size() - 1) {
				arr += '"';
			}
			arr += ']';
		}
		q.fields->Parse(arr.c_str());
		if (q.fields->HasParseError()) {
			std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
			return false;
		}
	} else {
		std::cout << "PARSING ERROR: Expected 'index on', found '" << index << " " << on << "'." << std::endl;
		return false;
	}
	return true;
}

bool Parsing::Parser::show(Parsing::Query &q) {
	q.command = SHOW;
	std::string token = toLower(Parsing::Parser::sc.nextToken());
	if (!token.compare("projects")) {
		q.project = new std::string("__PROJECTS__");
	} else {
		std::cout << "PARSING ERROR: Expected 'projects', found '" << token << "." << std::endl;
		return false;
	}
	return true;
}

bool Parsing::Parser::update(Parsing::Query &q) {
	q.command = UPDATE;
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	std::string with = toLower(Parsing::Parser::sc.nextToken());
	if (with.compare("with")) {
		std::cout << "Expected 'with', found '" << with << "." << std::endl;
		return false;
	}
	std::string withJSON = Parsing::Parser::sc.nextJSON();
	q.with = new rapidjson::Document();
	q.with->Parse(withJSON.c_str());
	if (q.with->HasParseError()) {
		std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
		return false;
	}
	std::string where = toLower(Parsing::Parser::sc.nextToken());
	if (where.compare("where")) {
		std::cout << "Expected 'where', found '" << where << "." << std::endl;
		return false;
	}
	std::string whereJSON = Parsing::Parser::sc.nextJSON();
	q.where = new rapidjson::Document();
	q.where->Parse(whereJSON.c_str());
	if (q.where->HasParseError()) {
		std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
		return false;
	}
	if (limitPending()) {
		q.limit = Parsing::Parser::sc.nextInt();
	}
	return true;
}

bool Parsing::Parser::select(Parsing::Query &q) {
	q.command = SELECT;
	q.fields = fieldList();
	if (!q.fields) {
		return false;
	}
	std::string token = Parsing::Parser::sc.nextToken();
	if (toLower(token).compare("from")) {
		std::cout << "PARSING ERROR: Expected 'from', found " << token << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	std::string where = toLower(Parsing::Parser::sc.nextToken());
	if (!where.compare("where")) {
		std::string whereJSON = Parsing::Parser::sc.nextJSON();
		q.where = new rapidjson::Document();
		q.where->Parse(whereJSON.c_str());
		if (q.where->HasParseError()) {
			std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
			return false;
		}
	} else {
		Parsing::Parser::sc.push_back(where);
	}
	if (limitPending()) {
		q.limit = Parsing::Parser::sc.nextInt();
	}
	return true;
}

bool Parsing::Parser::ddelete(Parsing::Query &q) {
	q.command = DELETE;
	std::string from = toLower(Parsing::Parser::sc.nextToken());
	if (from.compare("from")) {
		std::cout << "PARSING ERROR: Expected 'from', found " << from << "." << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	std::string where = toLower(Parsing::Parser::sc.nextToken());
	if (where.compare("where")) {
		std::cout << "PARSING ERROR: Expected 'where', found " << where << "." << std::endl;
		return false;
	}
	std::string whereJSON = Parsing::Parser::sc.nextJSON();
	q.where = new rapidjson::Document();
	q.where->Parse(whereJSON.c_str());
	if (q.where->HasParseError()) {
		std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
		return false;
	}
	if (limitPending()) {
		q.limit = Parsing::Parser::sc.nextInt();
	}
	return true;
}

bool Parsing::Parser::insert(Parsing::Query &q) {
	q.command = INSERT;
	std::string into = Parsing::Parser::sc.nextToken();
	if (toLower(into).compare("into")) {
		std::cout << "Expected 'into'.  Found '" << into << "." << std::endl;
		return false;
	}
	q.project = new std::string(Parsing::Parser::sc.nextToken());
	std::string with = Parsing::Parser::sc.nextToken();
	if (toLower(with).compare("with")) {
		std::cout << "Expected 'with'.  Found '" << with << "." << std::endl;
		return false;
	}
	char c = Parsing::Parser::sc.nextChar();
	// Read array
	if (c == '[' || c == '{') {
		Parsing::Parser::sc.push_back(1);
		std::string withJSON = Parsing::Parser::sc.nextJSON();
		q.with = new rapidjson::Document();
		q.with->Parse(withJSON.c_str());
		if (q.with->HasParseError()) {
			std::cout << "PARSING ERROR: Invalid JSON." << std::endl;
			return false;
		}
	} else {
		std::cout << "PARSING ERROR: Expected JSON array or object, found " << c << "..." << std::endl;
		return false;
	}
	return true;
}

rapidjson::Document *Parsing::Parser::fieldList() {
	rapidjson::Document *keys = new rapidjson::Document();
	keys->SetArray();
	bool done = false;
	while (!done) {
		if (aggregatePending()) {
			if (!aggregate(keys)) {
				std::cout << "PARSING ERROR: Malformed aggregate." << std::endl;
				return NULL;
			}
		} else {
			std::string field = Parsing::Parser::sc.nextToken();
			rapidjson::Value fieldVal;
			fieldVal.SetString(field.c_str(), keys->GetAllocator());
			keys->PushBack(fieldVal, keys->GetAllocator());
		}
		char next = Parsing::Parser::sc.nextChar();
		if (next != ',') {
			Parsing::Parser::sc.push_back(1);
			done = true;
		}
	}
	return keys;
}

bool Parsing::Parser::aggregate(rapidjson::Document *doc) {
	std::string funct = Parsing::Parser::sc.nextToken();
	char c = Parsing::Parser::sc.nextChar();
	if (c != '(') {
		std::cout << "PARSING ERROR: Expected open parenthesis, found '" << c << "'." << std::endl;
		return false;
	}
	std::string field = Parsing::Parser::sc.nextToken();
	c = Parsing::Parser::sc.nextChar();
	if (c != ')') {
		std::cout << "PARSING ERROR: Expected closed parenthesis, found '" << c << "'." << std::endl;
		return false;
	}

	std::string aggregate;

	int numAggregates = sizeof(Aggregates) / sizeof(std::string);
	for (int i=0; i<numAggregates; ++i) {
		if (!toLower(funct).compare(toLower(Aggregates[i]))) {
			aggregate = Aggregates[i];
		}
	}

	rapidjson::Value obj;
	obj.SetObject();

	rapidjson::Value functVal;
	functVal.SetString(aggregate.c_str(), doc->GetAllocator());
	
	rapidjson::Value fieldVal;
	fieldVal.SetString(field.c_str(), doc->GetAllocator());

	obj.AddMember("function", functVal, doc->GetAllocator());
	obj.AddMember("field", fieldVal, doc->GetAllocator());

	doc->PushBack(obj, doc->GetAllocator());
	return true;
}

bool Parsing::Parser::limitPending() {
	std::string limit = Parsing::Parser::sc.nextToken();
	bool found = false;
	if (!toLower(limit).compare("limit")) {
		found = true;
	} else {
		Parsing::Parser::sc.push_back(limit);
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
