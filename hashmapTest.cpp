#include "linhash.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>

int main(int argc, char **argv)
{
    clock_t start;
    HashTable::LinearHashTable<std::string,std::string> *ht = new HashTable::LinearHashTable<std::string,std::string>(16,16);
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
    return 0;
}
