#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "kvlist.h"

template <class T> class
HashMap {
private:
   KVList<T> **bucket;
   size_t size;

public:
   int
   getHash(std::string key)
      {
      std::hash<std::string> hash_fn;
      size_t h = hash_fn(key) % this->size;
      return h;
      }

   HashMap(const size_t size)
      {
      this->size = size;
      this->bucket = new KVList<T>*[size];
      }

   HashMap()
      {
      this->size = 255;
      this->bucket = new KVList<T>*[255];
      }

   void
   insert(std::string key, T value)
      {
      size_t hash = getHash(key);
      KVList<T> *pair = new KVList<T>(key, value);
      KVList<T> *b = this->bucket[hash];
      if (b == NULL)
         {
         this->bucket[hash] = pair;
         }
      else
         {
         while (b->next != NULL)
            {
            b = b->next;
            }
         b->next = pair;
         }
      }

   std::string
   get(std::string key)
      {
      size_t hash = getHash(key);
      KVList<T> *b = this->bucket[hash];
      while (b != NULL)
         {
         if (!b->key.compare(key))
            {
            return b->value;
            }
         b = b->next;
         }
      return std::string("NULL");
      }
};

#endif
