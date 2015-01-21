#ifndef DBASE_H
#define DBASE_H

#include <string>
#include <iostream>
#include <fstream>
#include "resultset.h"

#define unimplemented() std::cout << __FUNCTION__ << " Not Implemented" << std::endl

class
DBase
   {
   private:
   std::string fname;
   std::fstream file;

   

   public:
   DBase(const std::string fname)
      {
      this->fname = fname;
      this->file.open(fname);
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

   void
   commit()
      {
      unimplemented();
      }
   };

#endif
