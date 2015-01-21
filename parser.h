#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "dataset.h"

class
Parser
   {
   private:
   Scanner *scanner;

   public:
   Parser(Scanner *sc)
      {
      this->scanner = sc;
      }

   DataSet *
   getParseTree()
      {
      return NULL;
      }
   };
#endif
