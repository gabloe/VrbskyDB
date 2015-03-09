#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <iostream>
#include "Scanner.h"

namespace Parsing {
	const std::string Aggregates[] = {"AVG", "SUM", "STDEV" /*, TODO: Others. */};
	const std::string Commands[] = {"CREATE", "INSERT", "APPEND", "REMOVE", "SELECT", "DELETE" /*, TODO: Others. */};
	enum Command {
		CREATE,
		INSERT,
		APPEND,
		REMOVE,
		SELECT,
		DELETE
	};
	enum Aggregate {
		AVG,
		SUM,
		STDEV
	};
	struct List {
		std::string value;
		List *next;
		List(std::string value_): value(value_), next(NULL) {}
	};
	struct Query {
		Command command;
		Aggregate *aggregate;
		std::string *project;
		List *documents;
		std::string *key;
		std::string *value;
		Query(): aggregate(NULL), project(NULL), documents(NULL), key(NULL), value(NULL) {}
		void print() {
			std::cout << "Command: " << Commands[command] << std::endl;
			if (aggregate)
				std::cout << "Aggregate: " << Aggregates[*aggregate] << std::endl;
			if (project)
				std::cout << "Project: " << *project << std::endl;
			List *spot = documents;
			while (spot != NULL) {
				std::cout << "Document: " << spot->value << std::endl;
				spot = spot->next;
			}
			if (key)
				std::cout << "Key: " << *key << std::endl;
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
		bool where(Query &);
		void aggregate(Query &);
		List *idList();
		bool withDocumentsPending();
		bool withValuePending();
		bool wherePending();
		bool andValuePending();
		bool aggregatePending();
	};
}

#endif
