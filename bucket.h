#pragma once

#include <string>
#include <iostream>
#include <algorithm>

template <class T, class U>
class Bucket
   {
   private:
   T key;
   U value;
   Bucket<T, U> *next;
   Bucket<T, U> *prev;
   bool deleted;   
   size_t count;

   public:
   Bucket() : Bucket(NULL, NULL) {}

   Bucket(T key, U value)
      {
      this->key = key;
      this->value = value;
      this->next = NULL; 
      this->prev = NULL;
      this->deleted = false;
      this->count = 1;
      }

   bool
   isDeleted()
      {
      return this->deleted;
      }

   bool
   same(T key)
      {
      if (this->deleted)
         {
         return false;
         }

      return this->key == key;
      }

   void
   remove()
      {
      this->deleted = true;
      }

   Bucket<T, U> *
   getNext()
      {
      return this->next;
      }

   void
   setNext(Bucket<T, U> *n)
      {
      this->next = n;
      }

   void
   setPrev(Bucket<T, U> *p)
      {
      this->prev = p;
      }

   void
   setCount(size_t c)
      {
      this->count = c;
      }

   size_t
   getCount()
      {
      return this->count;
      }

   T
   getKey()
      {
      return this->key;
      }

   U
   getValue()
      {
      return this->value;
      }
   };
