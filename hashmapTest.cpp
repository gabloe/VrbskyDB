#include "hashtable.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>

int main(int argc, char **argv)
{
    if (argc != 2)
       {
       std::cout << "Usage: " << argv[0] << " <chain | linear | dynamic | probing>" << std::endl;
       exit(1);
       }

    HashTable::ChainingHashTable<std::string,std::string> *ht;
    if (!strcmp(argv[1], "chain"))
       {
       ht = new HashTable::ChainingHashTable<std::string,std::string>();
       }
    else if (!strcmp(argv[1], "dynamic"))
       {
       ht = new HashTable::DynamicChainingHashTable<std::string,std::string>();
       }
    else if (!strcmp(argv[1], "linear"))
       {
       ht = new HashTable::LinearHashTable<std::string,std::string>();
       }
    else if (!strcmp(argv[1], "probing"))
       {
       ht = new HashTable::DynamicProbingHashTable<std::string, std::string>();
       }
    else if (!strcmp(argv[1], "robinhood"))
       {
       ht = new HashTable::DynamicRobinHoodHashTable<std::string, std::string>();
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
    std::cout << "Empty buckets: " << ht->countEmpty() << std::endl;
    return 0;
}
