#include <math.h>
#include <stdexcept>
#include "bucket.h"
#include <iostream>

namespace HashTable
{
template <class T, class U>
class LinearHashTable
   {
   private:
   // Constants
   const size_t DEFAULT_BUCKETS = 16;
   const size_t DEFAULT_BUCKET_SIZE = 16;

   // Linear hashing -- # buckets (n), split pointer (s), level (l)
   size_t n;
   size_t s;
   size_t l;

   // Array of buckets
   Bucket<T, U> **buckets;

   // Size info
   size_t num_buckets;
   size_t num_items;
   size_t bucket_size;

   size_t
   hash(T key)
      {
      std::hash<T> h;
      size_t hash = h(key);

      size_t h0 = hash % (size_t)(this->n * pow(2, this->l));
      if (h0 < this->s)
         {
         // h1
         return hash % (size_t)(this->n * pow(2, this->l+1));; 
         }

      return h0; 
      }

   void
   split()
      {
      // Overflow!
      Bucket<T, U> *overflow = this->buckets[this->s];
      this->buckets[this->s] = NULL;
      Bucket<T, U> **newArr = new Bucket<T, U>*[this->num_buckets+1];
      std::copy(this->buckets, this->buckets+this->num_buckets, newArr);
      newArr[num_buckets++] = NULL;
      delete[] this->buckets;
      this->buckets = newArr;

      if (this->s >= this->n * pow(2, this->l))
         {
         this->s = 0;
         this->l++;
         }
      else
         {
         this->s++; 
         }
        
      // Rehash overflow spot
      while (overflow != NULL)
         {
         if (overflow->isDeleted())
            {
            overflow = overflow->getNext();
            continue;
            }
         // Insert the item into a new spot
         size_t h2 = hash(overflow->getKey());
         if (this->buckets[h2] == NULL)
            {
            this->buckets[h2] = new Bucket<T, U>(overflow->getKey(), overflow->getValue());
            } 
         else
            {
            Bucket<T, U> *tmp = new Bucket<T, U>(overflow->getKey(), overflow->getValue());
            tmp->setNext(this->buckets[h2]);
            this->buckets[h2]->setPrev(tmp);
            tmp->setCount(this->buckets[h2]->getCount()+1);
            this->buckets[h2] = tmp;
            }
         overflow = overflow->getNext();
         }
      delete overflow;
      }

   public:
   LinearHashTable(size_t mbs, size_t n)
      {
      this->n = n; 
      this->num_buckets = n;
      this->num_items = 0;
      this->s = 0;
      this->l = 0;
      this->bucket_size = mbs;
      this->buckets = new Bucket<T, U>*[n];
      for (int i = 0; i < n; ++i)
         {
         this->buckets[i] = NULL;
         }
      }

   LinearHashTable() : LinearHashTable(DEFAULT_BUCKET_SIZE, DEFAULT_BUCKETS) {}

   void
   print()
      {
      for (int i=0; i<this->num_buckets; ++i)
         {
         std::cout << i << ": ";
         Bucket<T, U> *b = this->buckets[i];
         if (!b)
            {
            std::cout << "Empty!" << std::endl;
            }
         else
            {
            while (b)
               {
               std::cout << "(" << b->getKey() << ", " << b->getValue() << ") ";
               b = b->getNext();
               }
            std::cout << std::endl;
            }
         }
      }
 
   size_t
   countEmpty()
      {
      size_t c = 0;
      for (int i=0; i<this->num_buckets; ++i)
         {
         if (!this->buckets[i])
            c++;
         }
      return c;
      }

   size_t
   getSize()
      {
      return this->num_buckets;
      }

   U
   get(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      while (b != NULL)
         {
         if (b->same(key))
            {
            return b->getValue();
            }
         b = b->getNext();
         }
      throw std::runtime_error("Key not found.");
      }

   bool
   remove(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      while (b)
         {
         if (!b->getKey().compare(key)) 
            {
            b->remove();
            return true;
            }
         b = b->getNext();
         }
      return false;
      }

   void
   put(T key, U value)
      {
      size_t h = hash(key);
      if (this->buckets[h] == NULL)
         {
         this->buckets[h] = new Bucket<T, U>(key, value);
         }
      else
         {
         Bucket<T, U> *tmp = new Bucket<T, U>(key, value);
         tmp->setNext(this->buckets[h]);
         this->buckets[h]->setPrev(tmp);
         tmp->setCount(this->buckets[h]->getCount()+1);
         this->buckets[h] = tmp;
         if (this->buckets[h]->getCount() > this->bucket_size)
            {
            split();
            }
         }
      this->num_items++;
      }
   };
}
