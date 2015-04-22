#include <iostream>
#include <vector>
#include <stdexcept>

#include "Scanner.h"

namespace Parsing {
	char Scanner::nextChar() {
		SKIPWHITESPACE();
		size_t pos = spot++;
		return query.at(pos);
	}

	std::string Scanner::nextToken() {
		SKIPWHITESPACE();
        std::vector<char> token;
		//std::string token = "";
		while (spot < query.size()) {
			size_t pos = spot++;
			char t = query.at(pos);
			if (t == ';' || t == ',' || t == '=' || t == '.' || t == ':' || t == '(' || t == ')') {
				--spot;
				break;
			} else if (t == ' ') {
				break;
			}
            token.push_back( t );
			//token = token + t;
		}
		return std::string( &token[0] , token.size() );;
	}

	void Scanner::push_back(std::string val) {
		unSkipWhiteSpace();
		spot -= val.size();
	}

	void Scanner::push_back(size_t spots) {
		unSkipWhiteSpace();
		spot -= spots;
	}

	std::string Scanner::nextJSON() {
		SKIPWHITESPACE();
		size_t pos = spot;
		std::string result = "";
		spot++;
		char t = query.at(pos);
		bool matchSquare = false;
		bool matchBrace = false;
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
		result += t;
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
			result += t;
		}
		if (numOpen != numClosed) {
			throw std::runtime_error("SCAN ERROR: Unmatched braces.");
		}
		return result;
	}

	std::string Scanner::nextString() {
		SKIPWHITESPACE();
		int pos = spot;
		std::string result = "";
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
			result = result + t;
		}
		return result;
	}

	int Scanner::nextInt() {
		std::string token = nextToken();
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
