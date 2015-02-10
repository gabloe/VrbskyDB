#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept>

template <class T, class U>
struct Tuple
   {
   T key;
   U value;
   bool deleted;
   };

template <class T, class U>
class Bucket
   {
   private:
   Tuple<T, U> *data;
   size_t size;
   size_t spot;
   size_t num_deleted;
   Bucket<T, U> *chain;

   public:
   Bucket() : Bucket(32) {};
   Bucket(size_t n) : size(n), spot(0), num_deleted(0), chain(NULL) 
      {
      this->data = new Tuple<T, U>[n];
      }

   size_t
   count()
      {
      return this->spot;
      }

   bool
   insert(T key, U value)
      {
      if (this->num_deleted)
         {
         for (int i=0; i<size; ++i)
            {
            if (this->data[i].deleted)
               {
               this->data[i].deleted = false;
               this->data[i].key = key;
               this->data[i].value = value;
               break;
               }
            }
         num_deleted--;
         return true;
         }
      
      if (this->spot < this->size)
         {
         this->data[this->spot].key = key;
         this->data[this->spot].value = value;
         this->data[this->spot].deleted = false;
         this->spot = this->spot+1;
         return true;
         }
      else
         {
         if (!this->chain)
            {
            this->chain = new Bucket<T, U>(size);
            }
         this->chain->insert(key, value);
         return false;
         }
      }

   bool
   remove(T key)
      {
      for (int i=0; i<this->spot; ++i)
         {
         if (!this->data[i].deleted && this->data[i].key == key)
            {
            this->data[i].deleted = true;
            this->num_deleted++;
            return true;
            }
         }
      return false;
      }

   U
   get(T key)
      {
      for (int i=0; i<this->size; ++i)
         {
         if (!this->data[i].deleted && this->data[i].key == key)
            {
            return this->data[i].value; 
            }
         }
      if (this->chain)
         {
         return this->chain->get(key);
         }
      throw std::runtime_error("Key not found.");
      }

   Bucket<T, U>*
   getNext()
      {
      return this->chain;
      }

   Tuple<T, U>*
   getData()
      {
      return this->data;
      }

   size_t
   getSize()
      {
      return this->size;
      }

   };
