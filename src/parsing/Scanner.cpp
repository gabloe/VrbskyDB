#include <iostream>
#include <vector>
#include <stdexcept>

#include "Scanner.h"

#include <cstdlib>

#define append(BUF,POS,VAL) {           \
    if(POS==len) {                      \
        len = 2 * len;                  \
        BUF = (char*)realloc(BUF,len);  \
    }                                   \
    BUF[POS] = VAL;                     \
    ++POS;                              \
}

namespace Parsing {
	char Scanner::nextChar() {
		SKIPWHITESPACE();
		size_t pos = spot++;
		return query.at(pos);
	}

	//std::string Scanner::nextToken() {
	const char* Scanner::nextToken() {
		SKIPWHITESPACE();
        int buf_pos = 0;
		while (spot < query.size()) {
			size_t pos = spot++;
			char t = query.at(pos);
			if (t == ';' || t == ',' || t == '=' || t == '.' || t == ':' || t == '(' || t == ')') {
				--spot;
				break;
			} else if (t == ' ') {
				break;
			}

            append(buffer,buf_pos,t);
		}
        // Null terminate
        buffer[buf_pos] = '\0';
        return buffer;
		//return std::string( buffer, buf_pos );
	}

	void Scanner::push_back(std::string val) {
		unSkipWhiteSpace();
		spot -= val.size();
	}

	void Scanner::push_back(size_t spots) {
		unSkipWhiteSpace();
		spot -= spots;
	}

	//std::string Scanner::nextJSON() {
	const char* Scanner::nextJSON() {
		SKIPWHITESPACE();

        int buf_pos = 0;

		size_t pos = spot;
		char t = query.at(pos);
		bool matchSquare = false;
		bool matchBrace = false;

		spot++;

		if (t == '{') {
			matchBrace = true;
		} else if (t == '[') {
			matchSquare = true;
		} else {
			std::cout << "Found " << t << std::endl;
			throw std::runtime_error("SCAN ERROR: Expected open brace.");
		}

		int numOpen = 1;
		int numClosed = 0;

        append( buffer , buf_pos , t );

		while (numOpen > numClosed && spot < query.size()) {
			pos = spot;
			spot++;
			t = query.at(pos);
			if (  (t == '{' && matchBrace) ||
			      (t == '[' && matchSquare) ) {
				numOpen++;
			}
			if (  (t == '}' && matchBrace) ||
			      (t == ']' && matchSquare) ) {
				numClosed++;
			}

            append( buffer , buf_pos , t );

		}
		if (numOpen != numClosed) {
			throw std::runtime_error("SCAN ERROR: Unmatched braces.");
		}
        buffer[buf_pos] = '\0';
        return buffer;
		//return std::string( buffer , buf_pos );;
	}

	//std::string Scanner::nextString() {
	const char* Scanner::nextString() {
		SKIPWHITESPACE();
		int pos = spot;
        int buf_pos = 0;
		spot++;
		char t = query.at(pos);
		if (t != '\"') {
			throw std::runtime_error("SCAN ERROR: Expected double quote.");
		}
		while (spot < query.size()) {
			pos = spot;
			spot++;
			char t = query.at(pos);
			if (t == '\"') {
				break;
			}
			if (spot == query.size()) {
				throw std::runtime_error("SCAN ERROR: Expected double quote.");
			}

            append(buffer,buf_pos,t);
		}
        buffer[buf_pos] = '\0';
        return buffer;
        //return std::string( buffer , buf_pos );
	}

	int Scanner::nextInt() {
		std::string token(nextToken());
		return atoi(token.c_str());
	}

	void Scanner::skipWhiteSpace() {
		char t;
		while (spot < query.size()) {
			int pos = spot;
			spot++;
			t = query.at(pos);
			if (t == ' ') {
				continue;
			} else {
				break;
			}
		}
		spot--;
	}

	void Scanner::unSkipWhiteSpace() {
		char t;
		while (spot > 1) {
			spot--;
			int pos = spot;
			t = query.at(pos);
			if (t == ' ') {
				continue;
			} else {
				break;
			}
		}
		spot++;
	}

}
