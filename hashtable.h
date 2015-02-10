/*
 *  Collection of hash table implementations.
 *  Written by Gabriel Loewen, 2015
 */

#include <math.h>
#include <stdexcept>
#include "bucket.h"
#include <iostream>
#include <vector>

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
   std::vector<Bucket<T, U>*> *buckets;
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
      this->buckets = new std::vector<Bucket<T, U>*>();
      for (int i=0; i<num_buckets; ++i)
         {
         this->buckets->push_back(new Bucket<T, U>());
         }
      }

   virtual ~ChainingHashTable()
      {
      }

   virtual bool
   put(T key, U value)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets->at(h);
      bool res = b->insert(key, value);
      this->num_items++;
      return res;
      }

   virtual U
   get(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets->at(h);
      return b->get(key);
      }

   virtual bool
   remove(T key)
      {
      size_t h = hash(key);
      Bucket<T, U> *b = this->buckets->at(h);
      return b->remove(key);
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

   };

/*
 *  Chaining hash table implementation that grows dynamically when the load factor becomes too large.
 */
template<class T, class U>
class DynamicChainingHashTable : public ChainingHashTable<T, U>
   {
   private:
   static const size_t DEFAULT_BUCKETS = 1024;
   float load_factor;

   void
   rehash()
      {
      size_t oldsize = ChainingHashTable<T, U>::num_buckets;
      ChainingHashTable<T, U>::num_buckets *= 2;
      std::vector<Bucket<T, U>*> *old = ChainingHashTable<T, U>::buckets;
      std::vector<Bucket<T, U>*> *buckets = new std::vector<Bucket<T, U>*>(); 
      for (int i=0; i<ChainingHashTable<T, U>::num_buckets; ++i)
         {
         buckets->push_back(new Bucket<T, U>());
         }
      ChainingHashTable<T, U>::buckets = buckets;
      ChainingHashTable<T, U>::num_items = 0;
      for (int i=0; i<oldsize; ++i)
         {
         Bucket<T, U> *b = old->at(i);
         while (b)
            {
            Tuple<T, U> *tuple = b->getData();
            for (int j=0; j<b->count(); ++j)
               {
               if (tuple[j].deleted) continue;
               ChainingHashTable<T, U>::put(tuple[j].key, tuple[j].value);
               }
            b = b->getNext();
            }
         }
      delete old;
      }

   public:
   DynamicChainingHashTable() : ChainingHashTable<T, U>(DEFAULT_BUCKETS){};
   DynamicChainingHashTable(const size_t n) : ChainingHashTable<T, U>(n){};

   bool
   put(T key, U value)
      {
      bool h = ChainingHashTable<T, U>::put(key, value);
      if (!h)
         {
         rehash();
         }
      return h;
      }

   };

template <class T, class U>
class LinearHashTable : public ChainingHashTable<T, U>
   {
   private:
   // Constants
   static const size_t DEFAULT_BUCKETS = 4096;

   // Linear hashing specific -- initial buckets (n), # split pointer (s), level (l)
   size_t n;
   size_t s;
   size_t l;

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
      Bucket<T, U> *overflow = ChainingHashTable<T, U>::buckets->at(this->s);
      ChainingHashTable<T, U>::buckets->at(this->s) = new Bucket<T, U>();
      ChainingHashTable<T, U>::buckets->push_back(new Bucket<T, U>());
      ChainingHashTable<T, U>::num_buckets++;
      if (this->s >= this->n * pow(2, this->l))
         {
         this->s = 0;
         this->l++;
         }
      else
         {
         this->s++; 
         }
      
      Bucket<T, U> *spot = overflow;
      // Rehash overflow spot
      while (spot)
         {
         Tuple<T, U> *data = spot->getData();
         for (int i=0; i<spot->count(); ++i)
            {
            if (data[i].deleted) continue;
            size_t h2 = hash(data[i].key);
            this->buckets->at(h2)->insert(data[i].key, data[i].value);
            }
         spot = spot->getNext();
         }
      delete overflow;
      }

   public:
   LinearHashTable(size_t n_) : ChainingHashTable<T, U>(n_), n(n_), s(0), l(0){};

   LinearHashTable() : LinearHashTable(DEFAULT_BUCKETS) {};

   bool
   put(T key, U value)
      {
      bool res = ChainingHashTable<T, U>::put(key, value);
      if (!res)
         {
         split();
         }
      return res;
      }
   };

template<class T, class U>
class ProbingHashTable : public ChainingHashTable<T, U>
   {
   public:
   ProbingHashTable() : ChainingHashTable<T, U>() {};
   ProbingHashTable(size_t n) : ChainingHashTable<T, U>(n) {};

   virtual ~ProbingHashTable() {};

   virtual size_t
   hash(T key)
      {
      std::hash<T> h;
      size_t hash = h(key);
      return hash % this->num_buckets;
      }

   virtual U
   get(T key)
      {
      size_t h = hash(key);
      int i=0;
      while (h+i<this->num_buckets)
         {
         Bucket<T, U> *b = this->buckets->at(h+i);
         try
            {
            return b->get(key);
            }
         catch (std::exception &e)
            {
            // Try next bucket.
            }         
         ++i;
         }
      throw std::runtime_error("Key not found.");
      }

   virtual bool
   put(T key, U value)
      {
      size_t h = hash(key);
      int i=0;
      while (h+i<this->num_buckets)
         {
         Bucket<T, U> *b = this->buckets->at(h+i);
         if (b->count() < b->getSize())
            {
            ChainingHashTable<T, U>::num_items++;
            return b->insert(key, value);
            }
         ++i;
         }
      throw std::runtime_error("Hash table is full.");
      }
   };

template<class T, class U>
class DynamicProbingHashTable : public ProbingHashTable<T, U>
   {
   protected:
   virtual void
   rehash()
      {
      std::vector<Bucket<T, U>*> *old = ProbingHashTable<T, U>::buckets;
      std::vector<Bucket<T, U>*> *tmp = new std::vector<Bucket<T, U>*>();
      size_t oldsize = ProbingHashTable<T, U>::num_buckets;
      ProbingHashTable<T, U>::num_buckets *= 2;
      for (int i=0; i<ProbingHashTable<T, U>::num_buckets; ++i)
         {
         tmp->push_back(new Bucket<T, U>());
         }
      ProbingHashTable<T, U>::buckets = tmp;
      ProbingHashTable<T, U>::num_items = 0;
      for (int i=0; i<oldsize; ++i)
         {
         Bucket<T, U> *b = old->at(i);
         Tuple<T, U> *tuple = b->getData();
         for (int i=0; i<b->count(); ++i)
            {
            if (tuple[i].deleted) continue;
            put(tuple[i].key, tuple[i].value);
            }
         }
      delete old;
      }

   public:
   bool
   put(T key, U value)
      {
      try
         {
         return ProbingHashTable<T, U>::put(key, value);
         }
      catch (std::exception &e)
         {
         rehash();
         return put(key, value);
         }
      }
   };
}
