#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "kvlist.h"

#define DEFAULT_SIZE 255

template <class T> class
HashMap {
private:
   KVList<T> **bucket;
   size_t size;

public:
   size_t
   getHash(std::string key)
      {
      std::hash<std::string> hash_fn;
      return hash_fn(key) % this->size;
      }

   HashMap(const size_t size)
      {
      this->size = size;
      this->bucket = new KVList<T>*[size];
      }

   HashMap()
      {
      this->size = DEFAULT_SIZE;
      this->bucket = new KVList<T>*[DEFAULT_SIZE];
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
      throw std::runtime_error("Key not found."); 
      }
};

#endif
