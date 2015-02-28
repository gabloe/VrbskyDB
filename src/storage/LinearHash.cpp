#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iterator>
#include <ctime>
#include <fstream>
#include <limits>

#ifdef _MSC_VER

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#else
#include <stdint.h>
#endif

const uint64_t INF = std::numeric_limits<uint64_t>::max();


#define DoSort false

bool print = true;

// Helper Functions

// If the test fails we print out the message and then exit
void Assert(std::string msg, bool test) {
	if (!test) {
		std::cerr << msg << std::endl;
		exit(-1);
	}
}

// Given an array d, we swap elements i and j
template <typename K>
void swap(K *d, uint64_t i, uint64_t j) {
	K t = d[i];
	d[i] = d[j];
	d[j] = t;
}

// Given an array d, we copy the contents of d[src] 
// to d[dest].  Anything in det is lost.
template <typename K>
void move(K *d, uint64_t dest, uint64_t src) {
	d[dest] = d[src];
}

// Linearly search through the array s from start to end for
// the value v.  If found return its index, otherwise return
// INF
template <typename K>
uint64_t linearSearch(const K* s, K v, uint64_t start, uint64_t end) {
	while (start + 7 < end) {

		if (s[start + 0] == v) return start + 0;
		if (s[start + 1] == v) return start + 1;
		if (s[start + 2] == v) return start + 2;
		if (s[start + 3] == v) return start + 3;
		if (s[start + 4] == v) return start + 4;
		if (s[start + 5] == v) return start + 5;
		if (s[start + 6] == v) return start + 6;
		if (s[start + 7] == v) return start + 7;

		start += 8;
	}
	while (start < end) {
		if (s[start] == v) {
			return start;
		}
		++start;
	}
	return INF;
}

// Given a sorted array we use binary search to search for
// the value v and return its position.  If not found we
// return INF
template <typename K>
uint64_t binarySearch(const K* s, K v, uint64_t left, uint64_t right) {

	while (left < right) {
		uint64_t mid = (left + right) / 2;
		if (s[mid] == v) {
			return mid;
		} else if (s[mid] < v) {	//  [left ... mid .V.. right]
			left = mid + 1;
		} else {						//  [left .V. mid .... right]
			right = mid;
		}
	}

	return INF;
}

// Try to convert a value to a string
template <typename T>
std::string to_string(T v) {
	std::ostringstream ss;
	ss << v;
	return ss.str();
}

template <typename K>
uint64_t search(K* s, K v, uint64_t start, uint64_t end) {
	if (DoSort) {
		return binarySearch(s, v, start, end);
	} else {
		return linearSearch(s, v, start, end);
	}
}

namespace DataStructures {

	template <typename T>
	class LinearHash {

		// Hard-coded defaults
		static const uint64_t NUM_BUCKETS = 100;
		static const uint64_t NUM_ELEMENTS = 32;

		// Forward declaration
		class Bucket;
		class MyIterator;

	public:
		LinearHash() {
			init();
		}

		LinearHash(uint64_t num_buckets) {
			num_buckets_ = num_buckets;
			init();
		}

		LinearHash(uint64_t num_buckets, uint64_t num_elements) {
			num_buckets_ = num_buckets;
			num_elements_ = num_elements;

			init();
		}

		~LinearHash() {
			for (uint64_t i = 0; i < num_buckets_; ++i) {
				if (buckets_[i] != NULL) {
					delete buckets_[i];
				}
			}
			delete[] buckets_;
		}

		// Given a key and value we place it in the hash table
		void put(uint64_t key, T *value) {

			// Get the bucket to place into
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);

			// If full we need to "split"
			if (b->full()) {

				expand();		// Grow the hash table
				split();		// split the bucket at S

				if (num_splits_ == num_buckets_) {	// reset if we doubled
					num_splits_ = 0;
					num_buckets_ *= 2;
				}

				// Might be in a different bucket so fetch again
				index = computeIndex(key);
				b = getBucket(index);

			}

			b->put(key, value);
			++num_items_;	// How many items in the table
		}

		T *get(uint64_t key) {
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);
			if (b == NULL) {
				return NULL;
			}
			return b->get(key);
		}

		T *remove(uint64_t key) {
			uint64_t index = computeIndex(key);
			Bucket *b = getBucket(index);
			if (b == NULL) {
				return NULL;
			}
			return b->remove(key);
		}

		bool contains(uint64_t key) {
			return get(key) != NULL;
		}

		uint64_t bucket_count() {
			return num_buckets_;
		}

		uint64_t split_count() {
			return num_splits_;
		}

		uint64_t bucket_size() {
			return num_elements_;
		}

		uint64_t count() {
			return num_items_;
		}

		MyIterator begin() {
			return MyIterator(buckets_, num_buckets_, num_splits_);
		}

		const MyIterator &end() {
			return end_;
		}

	private:

		void expand() {
			// If actually full
			if (num_splits_ == 0) {
				Bucket **r = new Bucket*[2 * num_buckets_];				// allocate new bucket
				Assert("Error: could not grow Linear Hash Table", r != 0);

				memset(r, 0, sizeof(Bucket*)* 2 * num_buckets_);

				for (uint64_t i = 0; i < num_buckets_; ++i) {
					r[i] = buckets_[i];
				}
				delete[] buckets_;
				buckets_ = r;
			}
		}

		void split() {
			const uint64_t second = num_buckets_ + num_splits_;
			Bucket *prev = getBucket(num_splits_);
			Bucket *next = getBucket(second);
			++num_splits_;

			while (prev != NULL) {

				uint64_t unmoved = 0;
				uint64_t end = prev->count_;

				for (uint64_t i = 0; i < end; ++i) {

					uint64_t idx = computeIndex(prev->keys_[i]);

					if (idx == second) {		// Move to other

						next->append(prev->keys_[i], prev->values_[i]);
						if (next->full()) {
							next->chain_ = new Bucket(num_elements_);
							next = next->chain_;
						}
						prev->values_[i] = NULL;
					} else {
						if (unmoved != i) {	// If not in their place already, then move
							prev->keys_[unmoved] = prev->keys_[i];
							prev->values_[unmoved] = prev->values_[i];
						}
						++unmoved;	// Didn't move so count
					}
				}
				prev->count_ = unmoved;
				prev = prev->chain_;
			}

			prev = getBucket(num_splits_);
			next = getBucket(second);

			prev->compact();
			next->compact();


		}

		void init() {
			buckets_ = new Bucket*[num_buckets_];
			memset(buckets_, 0, sizeof(Bucket*)* num_buckets_);
		}

		uint64_t computeIndex(uint64_t key) {
			uint64_t m = key % num_buckets_;
			if (m < num_splits_) {
				m = key % (2 * num_buckets_);
			}
			return m;
		}

		Bucket *getBucket(uint64_t index) {
			if (buckets_[index] == NULL) {
				buckets_[index] = new Bucket(num_elements_);
			}
			return buckets_[index];
		}

		class MyTuple {
		public:
			uint64_t key;
			T* value;

			MyTuple(uint64_t k, T* v) {
				key = k;
				value = v;
			}

			uint64_t getKey() {
				return key;
			}

			T* getValue() {
				return value;
			}
		};

		class MyIterator {
		private:
			Bucket **buckets;
			Bucket *bucket;
			uint64_t b_pos;
			uint64_t pos;
			uint64_t total;
			MyIterator end();

		public:

			MyIterator() {
				buckets = NULL;
				bucket = NULL;
				b_pos = pos = total = 0;
			}

			MyIterator(Bucket **bs, uint64_t num_buckets, uint64_t splits) {

				buckets = bs;
				bucket = bs[0];

				pos = b_pos = 0;

				total = num_buckets + splits;
				while (pos < total && (bucket == NULL || bucket->count_ == 0) ) {
					bucket = bs[++pos];
				}

				if (pos == total) {
					buckets = NULL;
					bucket = NULL;
					b_pos = pos = total = 0;
				}
				

			}

			MyTuple operator*() {
				MyTuple ret(bucket->keys_[b_pos], bucket->values_[b_pos]);
				++b_pos;
				return ret;
			}

			MyIterator &operator++() {
				while (pos < total) {
					// Search current bucket for item
					while (bucket != NULL) {
						if (bucket->count_ == b_pos) {		// Next bucket
							b_pos = 0;
							bucket = bucket->chain_;
						} else {							// Found a spot!
							return *this;
						}
					}
					++pos;
					bucket = buckets[pos];
				}

				buckets = NULL;
				bucket = NULL;
				pos = total = b_pos = 0;
				return *this;
			}

			MyIterator operator++(int) {
				return this++;
			}

			bool operator==(const MyIterator &other) const {
				return buckets == other.buckets && pos == other.pos && b_pos == other.b_pos;
			}

			bool operator!=(const MyIterator &other) const {
				return !(*this == other);
			}
		};

		class Bucket {

		public:
			// Global values
			uint64_t total_elements_;
			uint64_t total_count_;

			// Local values
			uint64_t num_elements_;
			uint64_t count_;

			// Data
			uint64_t *keys_;
			T **values_;

			// Chain
			Bucket *chain_;

			Bucket(uint64_t num_elements) {

				total_elements_ = num_elements_ = num_elements;

				total_count_ = count_ = 0;

				// Data
				keys_ = new uint64_t[num_elements_];
				values_ = new T*[num_elements_];

				memset(values_, 0, sizeof(T*)* num_elements_);

				chain_ = NULL;

			}

			~Bucket() {

				count_ = 0;
				num_elements_ = 0;

				if (keys_) {
					delete[] keys_;
					delete[] values_;
				}

				if (chain_) {
					delete chain_;
				}

				chain_ = NULL;
				keys_ = NULL;
				values_ = NULL;

			}

			Bucket *getBucket(uint64_t key, uint64_t &index) {
				Bucket *curr = this;

				while (curr != NULL) {
					index = search(curr->keys_, key, 0, curr->count_);

					// Check to see if we stop here
					if (index != INF || curr->chain_ == NULL) {
						return curr;
					}

					curr = curr->chain_;
				}
				return NULL;
			}


			// Special function only called when just created
			// Bucket and need to fill it right away
			void append(uint64_t key, T *value) {
				Bucket *curr = this;
				while (curr->full()) {
					if (curr->chain_ == NULL) {
						curr->chain_ = new Bucket(num_elements_);
					}
					curr = curr->chain_;
				}
				curr->sort(key, value);
				++curr->count_;
			}

			// Returns true if collision
			bool put(uint64_t key, T *value) {
				uint64_t index = INF;
				Bucket *bucket = this->getBucket(key, index);

				// If we found a previous key/value pair
				if (index != INF) {
					Assert("How...", false);
					if (bucket->values_[index] != NULL) {
						delete bucket->values_[index];	// Remove old
					}
					bucket->values_[index] = value;	// Add new
					return true;
				}

				// Last bucket
				if (bucket->full()) {
					bucket->chain_ = new Bucket(num_elements_);
					bucket = bucket->chain_;
				}

				bucket->append(key, value);
				return true;
			}

			// Search for a value given a key in this bucket
			T *get(uint64_t key) {
				uint64_t index = INF;
				Bucket *ret = getBucket(key, index);
				if (index == INF) {
					return NULL;
				}
				return ret->values_[index];
			}

			T *remove(uint64_t key) {
				// Find my bucket
				uint64_t index = INF;
				Bucket * b = getBucket(key, index);

				// We don't actually have it
				if (index == INF) {
					return NULL;
				}

				// Remove from bucket, keep sorted maybe
				--b->count_;
				T* v = b->values_[index];
				if (DoSort) {
					while (index < b->count_) {
						swap(b->keys_, index, index + 1);
						swap(b->values_, index, index + 1);
					}
				} else {
					swap(b->keys_, index, b->count_);
					swap(b->values_, index, b->count_);
				}
				return v;
			}

			bool full() {
				return count_ == num_elements_;
			}

			// Not implemented yet
			void compact() {
				Bucket *curr = this;
				while (curr->chain_ != NULL) {
					if (full()) {
						curr = curr->chain_;
						continue;
					}

					Bucket *t = curr->chain_;

					// If nothing in the next remove him
					if (curr->chain_->count_ == 0) {
						// Reconnect
						curr->chain_ = t->chain_;

						// Remove
						t->chain_ = NULL;
						delete t;
					} else {	// Copy what we can over
						--t->count_;
						uint64_t key = t->keys_[t->count_];
						T *value = t->values_[t->count_];

						curr->append(key, value);
					}
				}
			}

			// Just inserted at the end, sort
			void sort(uint64_t k, T* v) {
				uint64_t i = count_;
				if (DoSort) {
					// While our key is strictly smaller than the previous key
					while (i > 0 && keys_[i - 1] > k) {
						// Copy them up
						keys_[i] = keys_[i - 1];
						values_[i] = values_[i - 1];
						--i;
					}
				}
				keys_[i] = k;
				values_[i] = v;

			}

		};

		MyIterator end_;
		uint64_t num_buckets_ = NUM_BUCKETS;
		uint64_t num_elements_ = NUM_ELEMENTS;
		uint64_t count_ = 0;
		uint64_t num_splits_ = 0;
		uint64_t num_items_ = 0;
		Bucket **buckets_ = NULL;

	};

}

#define max(A,B) (A) > (B) ? (A) : (B)

void dumpToFile(std::string filename, DataStructures::LinearHash<std::string> &hash) {
	std::ofstream outfile(filename, std::ofstream::binary);
	// header

	uint64_t s_buff;
	const char* buffer = (const char*)&s_buff;

	s_buff = hash.bucket_count();
	outfile.write(buffer, sizeof(uint64_t));	// How many buckets

	s_buff = hash.split_count();
	outfile.write(buffer, sizeof(uint64_t));	// How many splits

	s_buff = hash.bucket_size();
	outfile.write(buffer, sizeof(uint64_t));	// How many elements

	s_buff = hash.count();
	outfile.write(buffer, sizeof(uint64_t));	// How many keys inserted

	// Data
	for (auto iter = hash.begin(); iter != hash.end(); ++iter) {
		auto pair = *iter;
		std::string *value = pair.getValue();
		//Assert("Somehow the value is null", value != NULL);
		uint64_t s_buff;
		const char* buffer = (const char*)&s_buff;

		// Print key
		s_buff = pair.getKey();
		outfile.write(buffer, sizeof(uint64_t));

		// Print length
		s_buff = value->length();
		outfile.write(buffer, sizeof(uint64_t));

		// Print string
		outfile.write(value->c_str(), value->length());

	}
	outfile.close();
}

void readFromFile(std::string filename, DataStructures::LinearHash<std::string> &hash) {
	std::ifstream infile(filename, std::ofstream::binary);

	uint64_t s_buff;
	const char* buffer = (const char*)&s_buff;

	uint64_t num_buckets, splits, num_elements, count;

	s_buff = hash.bucket_count();
	infile.read((char*)&num_buckets, sizeof(uint64_t));	// How many buckets

	s_buff = hash.split_count();
	infile.read((char*)&splits, sizeof(uint64_t));	// How many splits

	s_buff = hash.bucket_size();
	infile.read((char*)&num_elements, sizeof(uint64_t));	// How many elements

	s_buff = hash.count();
	infile.read((char*)&count, sizeof(uint64_t));	// How many keys inserted

	// I would hope this is large enough...
	char *str_buffer = new char[1024 * 1024];

	// Data
	for (uint64_t i = 0; i < count; ++i) {
		uint64_t key, length;

		infile.read((char*)&key, sizeof(uint64_t));
		infile.read((char*)&length, sizeof(uint64_t));
		infile.read(str_buffer, length);

		hash.put(key, new std::string(str_buffer, (size_t)length));
	}

	delete[] str_buffer;
	infile.close();
}


uint64_t MurmurHash64(const void * key, int len, unsigned int seed) {
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while (len >= 8) {
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4) {
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len) {
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;

	return h;
}

uint64_t herp(std::string &str) {
	uint64_t hash = 0;
	const uint64_t mul = 101;
	for (uint64_t i = 0; i < str.length(); ++i) {
		uint64_t c = str[i];
		hash = mul * hash + c;
	}
	return hash;
}

template <typename T>
uint64_t hash(const T* __ptr, size_t __clength)  {
	uint64_t __result
	= static_cast<uint64_t>(14695981039346656037ULL);
	const char* __cptr = reinterpret_cast<const char*>(__ptr);
	for (; __clength; --__clength)
	{
	__result ^= static_cast<size_t>(*__cptr++);
	__result *= static_cast<size_t>(1099511628211ULL);
	}
	return __result;
}
//std::hash<std::string> string_hash;

uint64_t str_hash(std::string &str) {
	//return MurmurHash64( str.c_str() , str.size() , 123654789 );
	//return herp(str);
	return hash( str.c_str() , str.size());
}

int main(void) {
	DataStructures::LinearHash<std::string> fromFile(1024, 16);
	DataStructures::LinearHash<std::string> myHash(1024, 16);

	int count = 0;

	std::string insert("ABCDEFGHI");
	std::string fetch("ABCDEFGHI");
	std::string file("ABCDEFGHI");


	std::string s1("DACIHEFBG");
	std::string s2("CGBHADIEF");

	std::cout << "The hash of DACIHEFBG is " << str_hash(s1) << std::endl;
	std::cout << "The hash of CGBHADIEF is " << str_hash(s2) << std::endl;

	clock_t start, end;
	start = std::clock();
	do {	// 362880 permutations
		uint64_t key = str_hash(insert);
		if (myHash.contains(key)) {
			std::string *s = myHash.get(key);
			uint64_t k = str_hash(*s);
			std::cout << "Herp: Collision: " << key << ", " << insert << ", " << k << ", " << *s << std::endl;
			return -1;
		}
		myHash.put(key, new std::string(insert));
	} while (std::next_permutation(insert.begin(), insert.end()));
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

	start = std::clock();
	do {
		++count;
		if (!myHash.contains(str_hash(fetch))) {
			std::cout << "ERROR: " << fetch << " at " << count << std::endl;
			return -1;
		}
	} while (std::next_permutation(fetch.begin(), fetch.end()));
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to fetch." << std::endl;
	std::cout << "Hashmap contains " << myHash.count() << " items" << std::endl;
	std::cout << "Total buckets: " << myHash.bucket_count() << std::endl;

	start = std::clock();
	dumpToFile("output.dat", myHash);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to write to disk." << std::endl;

	start = std::clock();
	readFromFile("output.dat", fromFile);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to read from disk." << std::endl;

	do {
		++count;
		uint64_t key = str_hash(file);
		if (!fromFile.contains(key)) {
			std::cout << "ERROR: " << file << " at " << count << std::endl;
			return -1;
		}
		delete fromFile.remove(key);
	} while (std::next_permutation(file.begin(), file.end()));

	return 0;
}
