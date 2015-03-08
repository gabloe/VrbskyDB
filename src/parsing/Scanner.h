#ifndef _SCANNER_H_
#define _SCANNER_H_

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
		std::string nextJSON();
		void push_back(std::string);
		void push_back(size_t);

	private:
		std::string query;
		size_t spot;
		void skipWhiteSpace();
	};
}

#endif
