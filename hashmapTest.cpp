#include "hashtable.h"
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <unordered_map>

int main(int argc, char **argv)
{
    if (argc != 3)
       {
       std::cout << "Usage: " << argv[0] << " <chain | linear | dynamic | probing> <size>" << std::endl;
       exit(1);
       }

    size_t size = atoi(argv[2]);
   
    if (!strcmp(argv[1], "std"))
       {
       std::unordered_map<std::string, std::string> *hmap = new std::unordered_map<std::string, std::string>(size);
       std::unordered_map<std::string, std::string> &ht = *hmap;
       clock_t start;
       start = std::clock();
       std::string s="ABCDEFGHI";
       do
          {
          ht[s]=s;
          }while(std::next_permutation(s.begin(),s.end()));

       std::cout << "Took " << 1000 * (float)(std::clock() - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

       start = std::clock();
       s="ABCDEFGHI";
       do
          {
          ht.at(s);
          }while(std::next_permutation(s.begin(),s.end()));
       std::cout << "Took " << 1000 * (float)(std::clock() - start) / CLOCKS_PER_SEC << "ms to look up." << std::endl;

       return 0;
       }

    HashTable::ChainingHashTable<std::string,std::string> *ht;
    if (!strcmp(argv[1], "chain"))
       {
       ht = new HashTable::ChainingHashTable<std::string,std::string>(size);
       }
    else if (!strcmp(argv[1], "dynamic"))
       {
       ht = new HashTable::DynamicChainingHashTable<std::string,std::string>(size);
       }
    else if (!strcmp(argv[1], "linear"))
       {
       ht = new HashTable::LinearHashTable<std::string,std::string>(size);
       }
    else if (!strcmp(argv[1], "probing"))
       {
       ht = new HashTable::DynamicProbingHashTable<std::string, std::string>(size);
       }
    else
       {
       exit(1);
       }

    clock_t start;
    start = std::clock();
    std::string s="ABCDEFGHI";
    do
       {
       ht->put(s, s);
       }while(std::next_permutation(s.begin(),s.end()));

    std::cout << "Took " << 1000 * (float)(std::clock() - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

    start = std::clock();
    s="ABCDEFGHI";
    do
       {
       ht->get(s);
       }while(std::next_permutation(s.begin(),s.end()));
    std::cout << "Took " << 1000 * (float)(std::clock() - start) / CLOCKS_PER_SEC << "ms to look up." << std::endl;
    
	std::cout << "Hashmap contains " << ht->getNumItems() << " items" << std::endl;
    std::cout << "Total buckets: " << ht->getSize() << std::endl;
    return 0;
}
