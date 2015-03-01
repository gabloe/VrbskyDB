#pragma once
#include <string>

#define SKIPWHITESPACE()							\
{										\
	skipWhiteSpace();							\
	if (spot == query.size())						\
	throw std::runtime_error("SCAN ERROR: Nothing left to scan!");		\
}

namespace Parsing {
	class Scanner {
	public:
		Scanner(std::string query_): query(query_), spot(0) {}
		char nextChar();
		std::string nextToken();
		std::string nextString();
		void push_back(std::string);
		void push_back(int);

	private:
		std::string query;
		int spot;
		void skipWhiteSpace();
	};
}
