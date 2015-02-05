#include <math.h>
#include <stdexcept>
#include "Bucket.h"
#include <iostream>

#define DEFAULT_BUCKETS 16
#define DEFAULT_BUCKET_SIZE 16

template <class U>
class LinearHashTable
   {
   private:
   size_t n;
   size_t s;
   size_t l;
   Bucket<std::string, U> **buckets;
   size_t num_buckets;
   size_t num_items;
   size_t bucket_size;

   size_t
   hash(std::string key)
      {
      std::hash<std::string> h;
      size_t hash = h(key);

      size_t h0 = hash % (size_t)(this->n * pow(2, this->l));
      size_t h1 = hash % (size_t)(this->n * pow(2, this->l+1));

      if (h0 < this->s)
         {
         return h1; 
         }

      return h0; 
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
      this->buckets = new Bucket<std::string, U>*[n];
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
         Bucket<std::string, U> *b = this->buckets[i];
         if (!b)
            {
            std::cout << "Empty!" << std::endl;
            }
         else
            {
            while (b)
               {
               std::cout << b->getValue() << " ";
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
   get(std::string key)
      {
      size_t h = hash(key);
      Bucket<std::string, U> *b = this->buckets[h];
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
   remove(std::string key)
      {
      size_t h = hash(key);
      Bucket<std::string, U> *b = this->buckets[h];
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
   put(std::string key, U value)
      {
      size_t h = hash(key);
      if (this->buckets[h] == NULL)
         {
         this->buckets[h] = new Bucket<std::string, U>(key, value);
         }
      else
         {
         this->buckets[h]->insert(key, value);
         if (this->buckets[h]->getCount() > this->bucket_size)
            {
            // Overflow!
            Bucket<std::string, U> *overflow = this->buckets[this->s];
            this->buckets[this->s] = NULL;
            Bucket<std::string, U> **newArr = new Bucket<std::string, U>*[this->num_buckets+1];
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
               // Insert the item into a new spot
               size_t h2 = hash(overflow->getKey());
               if (this->buckets[h2] == NULL)
                  {
                  this->buckets[h2] = new Bucket<std::string, U>(overflow->getKey(), overflow->getValue());
                  } 
               else
                  {
                  this->buckets[h2]->insert(overflow->getKey(), overflow->getValue());
                  }
               overflow = overflow->getNext();
               }
            delete overflow;
            }
         }
      this->num_items++;
      }
   };
