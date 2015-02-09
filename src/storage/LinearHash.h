#ifndef STORAGE_H_
#define STORAGE_H_

#include "util/Util.h"
#include "storage/Stream.h"

//
//	Implementation of a linear has on disk
//
//	Supports get/set
//
//


namespace Storage {

	class LinearHash {
		public:
			
			// Place data in the hash map
			void put( Util::count_t key , const char *data );
			
			// The returned data is owned by the caller
			char *get( Util::count_t key ) const;
			
			// true if key is found, false otherwise
			bool contains( Util::count_t key ) const;
			
			// Number of current buckets
			Util::count_t LinearHash::bucketCount() const;
			
			// How many elements a bucket can hold
			Util::count_t LinearHash::bucketSize() const;
			
			// How big a single element can be before it starts to chain
			Util::count_t LinearHash::elementSize() const;
			
			// How many total elements have been inserted
			Util::count_t LinearHash::count() const;
			
			// Given a file build a new hash table using linear hashing
			static LinearHash &build( const std::string &filename , Util::count_t N , Util::count_t K , Util::count_t E );
			// Load a hash table which uses linear hashing
			static LinearHash &load( const std::string &filename );
	};
	
}

#endif
