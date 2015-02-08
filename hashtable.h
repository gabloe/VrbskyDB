/*
 *  Collection of hash table implementations.
 *  Written by Gabriel Loewen, 2015
 */

#include <math.h>
#include <stdexcept>
#include "bucket.h"
#include <iostream>

namespace HashTable
{

/*
 *  Separate chaining hash table.
 */

template <class T, class U>
class ChainingHashTable
   {
   protected:
   // Array of buckets
   Bucket<T, U> **buckets;
   size_t num_buckets;
   size_t num_items;

   virtual size_t
   hash(T key)
      {
      std::hash<T> h;
      size_t hash = h(key);
      return hash % this->num_buckets;
      }
 
   public:
   ChainingHashTable() : ChainingHashTable(1024) {};

   ChainingHashTable(size_t n) : num_buckets(n), num_items(0)
      {
      this->buckets = new Bucket<T, U> *[n];
      for (int i=0; i<n; ++i)
         {
         this->buckets[i] = NULL;
         }
      }

   virtual ~ChainingHashTable()
      {
      delete[] this->buckets;
      }

   virtual size_t
   put(T key, U value)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      if (!b)
         {
         this->buckets[h] = new Bucket<T, U>(key, value);
         }
      else
         {
         Bucket<T, U> *tmp = new Bucket<T, U>(key, value);
         tmp->setNext(this->buckets[h]);
         tmp->setCount(this->buckets[h]->getCount()+1);
         this->buckets[h]->setPrev(tmp);
         this->buckets[h] = tmp;
         }
      this->num_items++;
      return h;
      }

   virtual U
   get(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      while (b)
         {
         if (b->same(key))
            {
            return b->getValue();
            }
         b = b->getNext();
         }
      throw std::runtime_error("Key not found.");
      }

   virtual bool
   remove(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets[h];
      while (b)
         {
         if (b->same(key)) 
            {
            b->remove();
            return true;
            }
         b = b->getNext();
         }
      return false;
      }

   virtual size_t
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

   virtual size_t
   getSize()
      {
      return this->num_buckets;
      }

   virtual size_t
   getNumItems()
      {
      return this->num_items;
      }

   virtual void
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

   };

/*
 *  Chaining hash table implementation that grows dynamically when the load factor becomes too large.
 */

template<class T, class U>
class DynamicChainingHashTable : public ChainingHashTable<T, U>
   {
   private:
   static constexpr float DEFAULT_LF = 0.75f;
   static const size_t DEFAULT_BUCKETS = 1024;
   float load_factor;

   void
   rehash()
      {
      size_t oldsize = ChainingHashTable<T, U>::num_buckets;
      ChainingHashTable<T, U>::num_buckets *= 2;
      Bucket<T, U> **old = ChainingHashTable<T, U>::buckets;
      Bucket<T, U> **buckets = new Bucket<T, U>*[ChainingHashTable<T, U>::num_buckets*2];
      for (int i=0; i<ChainingHashTable<T, U>::num_buckets; ++i)
         {
         buckets[i] = NULL;
         }
      ChainingHashTable<T, U>::buckets = buckets;
      ChainingHashTable<T, U>::num_items = 0;
      for (int i=0; i<oldsize; ++i)
         {
         Bucket<T, U> *b = old[i];
         while (b)
            {
            ChainingHashTable<T, U>::put(b->getKey(), b->getValue());
            b = b->getNext();
            }
         }
      delete[] old;
      }

   public:
   DynamicChainingHashTable() : ChainingHashTable<T, U>(DEFAULT_BUCKETS), load_factor(DEFAULT_LF) {};
   DynamicChainingHashTable(const size_t n) : ChainingHashTable<T, U>(n), load_factor(DEFAULT_LF) {};
   DynamicChainingHashTable(const double lf, const size_t n) : ChainingHashTable<T, U>(n), load_factor(lf) {};

   size_t
   put(T key, U value)
      {
      size_t h = ChainingHashTable<T, U>::put(key, value);
      if (ChainingHashTable<T, U>::num_items / ChainingHashTable<T, U>::num_buckets > this->load_factor)
         {
         rehash();
         }
      return h;
      }

   };

/*
 *  Linear hashing hash table implementation.  Extends the basic seperate chaining hash table.
 */

template <class T, class U>
class LinearHashTable : public ChainingHashTable<T, U>
   {
   private:
   // Constants
   static const size_t DEFAULT_BUCKETS = 1024;
   static const size_t DEFAULT_BUCKET_SIZE = 32;

   // Linear hashing specific -- initial buckets (n), # split pointer (s), level (l)
   size_t n;
   size_t s;
   size_t l;

   // Size info
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
      Bucket<T, U> *overflow = ChainingHashTable<T, U>::buckets[this->s];
      this->buckets[this->s] = NULL;
      Bucket<T, U> **newArr = new Bucket<T, U>*[ChainingHashTable<T, U>::num_buckets+1];
      std::copy(ChainingHashTable<T, U>::buckets, ChainingHashTable<T, U>::buckets+ChainingHashTable<T, U>::num_buckets, newArr);
      newArr[ChainingHashTable<T, U>::num_buckets++] = NULL;
      delete[] ChainingHashTable<T, U>::buckets;
      ChainingHashTable<T, U>::buckets = newArr;

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
            ChainingHashTable<T, U>::buckets[h2] = new Bucket<T, U>(overflow->getKey(), overflow->getValue());
            } 
         else
            {
            Bucket<T, U> *tmp = new Bucket<T, U>(overflow->getKey(), overflow->getValue());
            tmp->setNext(ChainingHashTable<T, U>::buckets[h2]);
            ChainingHashTable<T, U>::buckets[h2]->setPrev(tmp);
            tmp->setCount(ChainingHashTable<T, U>::buckets[h2]->getCount()+1);
            ChainingHashTable<T, U>::buckets[h2] = tmp;
            }
         overflow = overflow->getNext();
         }
      delete overflow;
      }

   public:
   LinearHashTable(size_t mbs, size_t n_) : ChainingHashTable<T, U>(n_), n(n_), s(0), l(0), bucket_size(mbs) {};

   LinearHashTable() : LinearHashTable(DEFAULT_BUCKET_SIZE, DEFAULT_BUCKETS) {};

   ~LinearHashTable(){}

   size_t
   put(T key, U value)
      {
      size_t h = ChainingHashTable<T, U>::put(key, value);
      if (ChainingHashTable<T, U>::buckets[h]->getCount() > this->bucket_size)
         {
         split();
         }
      return h;
      }
   };


/*
 *  TODO: open addressing hash table (linear probing, robin hood, etc.)
 */

template<class T, class U>
class ProbingHashTable
   {
   public:
   ProbingHashTable() {};
   virtual ~ProbingHashTable() {};
   };
}
