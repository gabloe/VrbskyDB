#ifndef DBASE_H
#define DBASE_H

#include <string>
#include <iostream>
#include <fstream>

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

   void
   query(const std::string query)
      {
      unimplemented();
      }

   void
   commit()
      {
      unimplemented();
      }

   std::string getFilename()
      {
      return this->fname;
      }

   };

#endif
