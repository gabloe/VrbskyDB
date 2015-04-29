#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <string>
#include <cstdlib>

#define SKIPWHITESPACE()							\
{										\
	skipWhiteSpace();							\
	if (spot == query.size())						\
	throw std::runtime_error("SCAN ERROR: Nothing left to scan!");		\
}

namespace Parsing {
	class Scanner {
	public:
		Scanner(std::string query_): query(query_), spot(0) {
            len = 512;
            buffer = (char*)malloc( len );
        }
        ~Scanner() {
            free(buffer);
        }
		char nextChar();
		const char* nextToken();
		const char* nextString();
		const char* nextJSON();
		int nextInt();
		void push_back(std::string);
		void push_back(size_t);

	private:
		std::string query;
		size_t spot;
		void skipWhiteSpace();
		void unSkipWhiteSpace();
        char *buffer;
        int len = 0;
	};
}

#endif
