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
   compare(T key)
      {
      return this->deleted || this->key.compare(key);
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

   size_t
   getCount()
      {
      return this->count;
      }

   void
   insert(T key, U value)
      {
      Bucket<T, U> *b = this;
      if (b->deleted == true)
         {
         this->key = key;
         this->value = value;
         }
      else if (b->next == NULL)
         {
         b->count++;
         b->next = new Bucket<T, U>(key, value);
         b->next->prev = b;
         }
      else
         {
         b->count++;
         b->next->insert(key, value);
         }
      }
   };
