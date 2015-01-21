#ifndef DBASE_H
#define DBASE_H

#include <string>
#include <iostream>
#include <fstream>
#include "resultset.h"
#include "dataset.h"
#include "scanner.h"

#define unimplemented() std::cout << __FUNCTION__ << " Not Implemented" << std::endl

class
DBase
   {
   private:
   std::string fname;
   std::fstream file;
   DataSet *data;

   DataSet *
   load(std::fstream& file)
      {
      Scanner *sc = new Scanner(file);
      while (sc->hasNext())
         {
         double s = sc->nextReal();
         std::cout << s << std::endl;
         }
      //Parser *parser = new Parser(sc);
      //DataSet *ds = parser->getParseTree();
      //return ds;
      return 0;
      }

   public:
   DBase(const std::string fname)
      {
      this->fname = fname;
      this->file.open(fname.c_str());
      this->data = load(this->file);
      this->file.close();
      }

   ~DBase()
      {
      this->commit();
      }

   std::string getFilename()
      {
      return this->fname;
      }

   ResultSet *
   query(const std::string query)
      {
      unimplemented();
      return NULL;
      }

   int
   commit()
      {
      unimplemented();
      return 0;
      }
   };
#endif
