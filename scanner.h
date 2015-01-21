#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>
#include <sstream>
#include <ctype.h>
#include <stdlib.h>

class
Scanner
   {
   private:
   std::fstream& file;

   char
   readChar()
      {
      char c;
      this->file.get(c);
      return c;
      }

   void
   skipWhitespace()
      {
      char c;
      while(this->file.get(c) && isspace(c))
         {
         continue;
         }
      this->file.putback(c);
      }

   public:
   Scanner(std::fstream& f) : file(f)
      {
      // Constructor
      }

   bool
   hasNext()
      {
      skipWhitespace();
      if (this->file.get() == -1)
         {
         return false;
         }
      this->file.unget();
      return true;
      }

   std::string
   next()
      {
      char c;
      std::string res;

      skipWhitespace();
      while (c = readChar())
         {
         if (isspace(c))
            {
            break;
            }
         res += c;
         }
      return res;
      }

   char
   nextChar()
      {
      skipWhitespace();
      return readChar();
      }

   std::string
   nextString()
      {
      char c;
      std::string res;

      skipWhitespace();
      c = readChar();
      if (c != '"')
         {
         std::cerr << "SCAN ERROR: Failed to read a string!" << std::endl;
         std::cerr << "Found `" << c << "' but expected `\"'" << std::endl;
         exit(-1);
         }

      res += c;
      while(c = readChar())
         {
         res += c;
         if (this->file.eof() || c == '"')
            {
            break;
            }
         }
      
      if (c != '"' && this->file.eof())
         {
         std::cerr << "SCAN ERROR: Failed to read a string!" << std::endl;
         std::cerr << "Reached end of file but expected `\"'" << std::endl;
         exit(-1);
         }

      return res;
      }

   int
   nextInt()
      {
      std::string s = next();
      std::stringstream ss(s);
      int i;
      ss >> i;
      ss >> std::ws;
      if (!ss.fail() && ss.eof())
         {
         return i;
         }
      else
         {
         std::cerr << "SCAN ERROR: Failed to read an integer!" << std::endl;
         exit(-1);
         }
      }

   double
   nextReal()
      {
      std::string s = next();
      std::stringstream ss(s);
      double d;
      ss >> d;
      ss >> std::ws;
      if (!ss.fail() && ss.eof())
         {
         return d;
         }
      else
         {
         std::cerr << "SCAN ERROR: Failed to read a real!" << std::endl;
         exit(-1);
         }
      }

   };

#endif
