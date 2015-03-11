#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <iostream>
#include "Scanner.h"

namespace Parsing {
	const std::string Aggregates[] = {"AVG", "MIN", "MAX", "SUM", "STDEV" /*, TODO: Others. */};
	const std::string Commands[] = {"CREATE", "INSERT", "APPEND", "REMOVE", "SELECT", "DELETE", "SHOW" /*, TODO: Others. */};
	enum Command {
		CREATE = 0,
		INSERT = 1,
		APPEND = 2,
		REMOVE = 3,
		SELECT = 4,
		DELETE = 5,
		SHOW   = 6
	};
	enum Aggregate {
		AVG   = 0,
		SUM   = 1,
		STDEV = 2
	};
	struct List {
		Aggregate *aggregate;
		std::string value;
		List *next;
		List(std::string value_): aggregate(NULL), value(value_), next(NULL) {}
		List(): aggregate(NULL), value(""), next(NULL) {}
	};
	struct Query {
		Command command;
		std::string *project;
		List *documents;
		List *keys;
		std::string *value;
		Query(): project(NULL), documents(NULL), keys(NULL), value(NULL) {}
		void print() {
			std::cout << "Command: " << Commands[command] << std::endl;
			if (project)
				std::cout << "Project: " << *project << std::endl;
			List *docspot = documents;
			while (docspot) {
				std::cout << "Document: " << docspot->value << std::endl;
				docspot = docspot->next;
			}
			List *keyspot = keys;
			while (keyspot) {
				std::cout << "Key: ";
				if (keyspot->aggregate) {
					std::cout << Aggregates[*(keyspot->aggregate)] << "(";
				}
				std::cout << keyspot->value;
				if (keyspot->aggregate) {
					std::cout << ")" << std::endl;
				} else {
					std::cout << std::endl;
				}
				keyspot = keyspot->next;
			}
			if (value)
				std::cout << "Value: " << *value << std::endl;
		}
	};
	class Parser {
	public:
		Parser(std::string q): sc(q) {}
		Parsing::Query* parse();
	private:
		Scanner sc;
		bool insert(Query &);
		bool append(Query &);
		bool remove(Query &);
		bool select(Query &);
		bool ddelete(Query &);
		bool create(Query &);
		bool show(Query &q);
		List *keyList();
		bool aggregate(List *);
		List *idList();
		bool withDocumentsPending();
		bool withValuePending();
		bool aggregatePending();
	};
}

#endif
