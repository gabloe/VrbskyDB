#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <iostream>
#include "Scanner.h"

namespace Parsing {
	const std::string Aggregates[] = {"AVG", "MIN", "MAX", "SUM", "STDEV" /*, TODO: Others. */};
	const std::string Commands[] = {"CREATE", "INSERT", "ALTER", "REMOVE", "SELECT", "DELETE", "SHOW" /*, TODO: Others. */};
	enum Command {
		CREATE = 0,
		INSERT = 1,
		ALTER = 2,
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
	template <typename T>
	struct List {
		Aggregate *aggregate;
		T value;
		List<T> *next;
		List<T>(T value_): aggregate(NULL), value(value_), next(NULL) {}
		List<T>(): aggregate(NULL), next(NULL) {}
		~List<T>() {
			if (aggregate) delete aggregate;
			if (next) delete next;
		}
		void append(T value_) {
			List<T> l = new List<T>();
			l->value = value_;
			if (!next) {
				l->next = next;
			}
			next = l;
		}
	};	
	struct Query {
		Command command;
		std::string *project;
		List<std::string> *documents;
		List<std::string> *keys;
		std::string *value;
		Query(): project(NULL), documents(NULL), keys(NULL), value(NULL) {}
		void print() {
			std::cout << "Command: " << Commands[command] << std::endl;
			if (project)
				std::cout << "Project: " << *project << std::endl;
			List<std::string> *docspot = documents;
			while (docspot) {
				std::cout << "Document: " << docspot->value << std::endl;
				docspot = docspot->next;
			}
			List<std::string> *keyspot = keys;
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
		bool alter(Query &);
		bool remove(Query &);
		bool select(Query &);
		bool ddelete(Query &);
		bool create(Query &);
		bool show(Query &q);
		List<std::string> *keyList();
		bool aggregate(List<std::string> *);
		List<std::string> *idList();
		bool withDocumentsPending();
		bool withValuePending();
		bool aggregatePending();
	};
}

#endif
