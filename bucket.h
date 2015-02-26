#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <vector>

template <class T, class U>
struct Tuple
   {
   T key;
   U value;
   Tuple(T key_, U value_) : key(key_), value(value_) {};
   Tuple(T key_) : key(key_) {};
   bool operator<(const Tuple<T, U> &a) const;
   bool operator==(const Tuple<T, U> &a) const;
   };

template <class T, class U>
bool Tuple<T, U>::operator<(const Tuple<T, U> &a) const
   {
   return key < a.key;
   }

template <class T, class U>
bool Tuple<T, U>::operator==(const Tuple<T, U> &a) const
   {
   return key == a.key;
   }

template <class T, class U>
class Bucket
   {
   private:
   size_t bucket_size;
   std::vector<Tuple<T, U>> data;
   bool overflow;

   public:
   Bucket() : bucket_size(32), overflow(false)
      {
      this->data.reserve(bucket_size);
      }

   size_t
   count()
      {
      return this->data.size();
      }

   bool
   isOverflow()
      {
      return this->overflow;
      }

   bool
   insert(T key, U value)
      {
      this->data.push_back(Tuple<T, U>(key, value));
      if (this->data.size() > this->bucket_size)
         {
         this->bucket_size *= 2;
         this->data.reserve(this->bucket_size);
         this->overflow = true;
         return false;
         }
      return true;
      }

   bool
   remove(T key)
      {
      typename std::vector<Tuple<T, U>>::iterator it = std::find(this->data.begin(), this->data.end(), Tuple<T, U>(key));
      if (it == this->data.end())
         {
         return false;
         }
      this->data.erase(it);
      return true; 
      }

   U
   get(T key)
      {
      typename std::vector<Tuple<T, U>>::iterator it = std::find(this->data.begin(), this->data.end(), Tuple<T, U>(key));
      if (it == this->data.end())
         throw std::runtime_error("Key not found.");
      Tuple<T, U> kv = *it;
      return kv.value; 
      }

   std::vector<Tuple<T, U>>
   getData()
      {
      return this->data;
      }

   size_t
   getSize()
      {
      return this->bucket_size;
      }

   };
