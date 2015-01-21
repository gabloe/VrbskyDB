#include "dbase.h"
#include <stdlib.h>
#include <iostream>

int
main(int argc, char *argv[])
   {
   if (argc < 2)
      {
      std::cerr << "Expected filename!" << std::endl;
      exit(1);
      }
   DBase *db = new DBase(std::string(argv[1]));
   std::cout << "Loading Database: `" << argv[1] << "'" << std::endl; 
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
      db->query(query);
      }
   delete db;
   return 0;
   }
