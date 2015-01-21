#include "dbase.h"
#include <stdlib.h>
#include <iostream>

int
main(int argc, char *argv[])
   {
   // Validate the number of arguments
   if (argc != 2)
      {
      std::cerr << "usage: " << argv[0] << " <FILENAME>" << std::endl;
      exit(1);
      }

   // Load the database
   DBase *db = new DBase(std::string(argv[1]));
   std::cout << "Loading Database: `" << argv[1] << "'" << std::endl; 

   // Interactive queries
   std::string query;
   while (1)
      {
      std::cout << ":";
      std::getline(std::cin, query);
      if (!query.compare("exit"))
         {
         break;
         }
      std::cout << "Query: " << query << std::endl;
      ResultSet *rs = db->query(query);
      //displayResults(rs);
      }

   // Destroy all the stuff
   delete db;
   return 0;
   }

void
displayResults(ResultSet *rs)
   {
   std::cout << "Put the results here!" << std::endl;
   }
