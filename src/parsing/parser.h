#include <string>
#include <iostream>
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
