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

   public:
   Bucket() : Bucket(8) {};
   Bucket(size_t n) : size(n), spot(0), num_deleted(0)
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
      
      bool normal = true;
      if (this->spot == this->size)
         {
         Tuple<T, U> *newArr = new Tuple<T, U>[this->size*4];
         std::copy(this->data, this->data+this->spot, newArr);
         this->size*=4;
         delete[] this->data;
         this->data = newArr;
         normal = false;
         }

      this->data[this->spot].key = key;
      this->data[this->spot].value = value;
      this->data[this->spot].deleted = false;
      this->spot++;

      return normal;
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
      throw std::runtime_error("Key not found.");
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
