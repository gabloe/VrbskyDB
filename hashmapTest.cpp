#include "linhash.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>

int main(int argc, char **argv)
{
    clock_t start;
    LinearHashTable<std::string> *ht = new LinearHashTable<std::string>(4,16);
    start = std::clock();
    std::string s="ABCDEFG";
    do
       {
       ht->put(s, s);
       }while(std::next_permutation(s.begin(),s.end()));

    std::cout << "Took " << 1000 * (float)(std::clock() - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;
    ht->print();
    return 0;
}
