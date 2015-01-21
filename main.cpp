#include "dbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int
main(int argc, char *argv[])
   {
   if (argc < 2)
      {
      fprintf(stderr, "Not enough arguments\n");
      exit(1);
      }
   char *fname = argv[1];
   DBase *db = new DBase(std::string(fname));
   std::cout << "Loading Database: `" << db->getFilename() << "'" << std::endl; 
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
