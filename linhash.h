#include <math.h>
#include <stdexcept>
#include "bucket.h"
#include <iostream>

#define DEFAULT_BUCKETS 16
#define LOAD_FACTOR 0.75

template <class T, class U>
class LinearHashTable
   {
   private:
   size_t n;
   size_t s;
   size_t l;
   double lf;
   Bucket<T, U> **buckets;
   size_t num_buckets;
   size_t num_items;

   void
   rehash()
      {
      Bucket<T, U> **b = this->buckets;
      size_t oldN = this->num_buckets;
      this->num_buckets = (size_t)pow(2,ceil(log(this->num_buckets)/log(2)));
      this->n = this->num_buckets;
      this->s = 0;
      this->l = 0;
      this->num_items = 0;
      this->buckets = new Bucket<T, U>*[this->n];

      for (int i=0; i<this->n; ++i)
         {
         this->buckets[i] = NULL;
         }

      for (int i=0; i<oldN; ++i)
         {
         Bucket<T, U> *tmp = b[i];
         while (tmp)
            {
            put(tmp->getKey(), tmp->getValue());
            tmp = tmp->getNext(); 
            }
         }
      delete b;
      }

   size_t
   hash(T key)
      {
      std::hash<T> h;
      size_t hash = h(key);

      if (hash % this->n * pow(2, this->l) < this->s)
         {
         return hash % this->n * pow(2, this->l+1);
         }

      return hash % this->n * pow(2, this->l);
      }

   public:
   LinearHashTable(double lf, size_t n)
      {
      this->n = n; 
      this->num_buckets = n;
      this->num_items = 0;
      this->s = 0;
      this->l = 0;
      this->lf = lf;
      this->buckets = new Bucket<T, U>*[n];
      for (int i = 0; i < n; ++i)
         {
         this->buckets[i] = NULL;
         }
      }

   LinearHashTable() : LinearHashTable(LOAD_FACTOR, DEFAULT_BUCKETS) {}
 
   U
   get(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      while (b != NULL)
         {
         if (!b->compare(key))
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
      // Rehash the map in case the number of elements exceeds the load factor.
      if (this->num_items >= this->lf * this->num_buckets)
         {
         rehash();
         }

      size_t h = hash(key);
      if (this->buckets[h] == NULL)
         {
         this->buckets[h] = new Bucket<T, U>(key, value);
         }
      else
         {
         // Overflow!
         this->buckets[h]->insert(key, value);
         Bucket<T, U> *overflow = this->buckets[this->s];
         this->buckets[this->s] = NULL;
         this->s++;
         Bucket<T, U> **newArr = new Bucket<T, U>*[this->num_buckets+1];
         std::copy(this->buckets, this->buckets+this->num_buckets, newArr);
         newArr[num_buckets++] = NULL;
         delete[] this->buckets;
         this->buckets = newArr;

         // Rehash overflow spot
         while (overflow != NULL)
            {
            // Insert the item into a new spot
            size_t h2 = hash(overflow->getKey());
            if (this->buckets[h2] == NULL)
               {
               this->buckets[h2] = new Bucket<T, U>(overflow->getKey(), overflow->getValue());
               } 
            else
               {
               this->buckets[h2]->insert(overflow->getKey(), overflow->getValue());
               }
            overflow = overflow->getNext();
            }

         if (this->s == this->n * pow(2, this->l))
            {
            this->s = 0;
            this->l++;
            }
         delete overflow;
         }
      this->num_items++;
      }
   };
