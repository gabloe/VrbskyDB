#ifndef _LIST_H_
#define _LIST_H_

#include <string>

template <class T> class
KVList {
public:
   KVList(KVList *chain, std::string key, T value)
      {
      this->next = chain;
      this->key = key;
      this->value = value;
      }

   KVList(std::string key, T value)
      {
      this->next = NULL;
      this->key = key;
      this->value = value;
      }

   KVList *next;
   std::string key;
   T value;
};

#endif
