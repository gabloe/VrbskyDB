#pragma once

#include <string>
#include <iostream>

template <class T, class U>
class Bucket
   {
   private:
   T key;
   U value;
   Bucket<T, U> *next;
   bool deleted;   

   public:
   Bucket() : Bucket(NULL, NULL) {}

   Bucket(T key, U value)
      {
      this->key = key;
      this->value = value;
      this->next = NULL; 
      this->deleted = false;
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

   void
   insert(T key, U value)
      {
      Bucket<T, U> *b = this;
      if (b == NULL)
         {
         *b = *(new Bucket<T, U>(key, value));
         }
      else if (b->deleted == true)
         {
         this->key = key;
         this->value = value;
         }
      else if (b->next == NULL)
         {
         b->next = new Bucket<T, U>(key, value);
         }
      else
         {
         b->next->insert(key, value);
         }
      }
   };
