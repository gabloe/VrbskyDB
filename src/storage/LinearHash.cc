
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


namespace Storage {

	const Util::count_t c_t = sizeof( Util::count_t );

	// Holds key/value pair.
	// Additional point in case needing to chain
	struct Element_t {
		Util::count_t key,length;
		const char* value;
		Element_t *chain;
	};
	
	// Bucket
	struct Bucket_t {
		Util::count_t numBuckets,elementSize;
		Element_t *elements;
		
	};

	
	// Linear Hashing header
	//
	//	Holds needed parameters for the linear hash as well as
	//	any extra useful information.
	//
	//	Allocates a total of 1024 bytes for the struct
	//
	struct LinearHash_t {
		Util::count_t numBuckets,bucketSize,elementSize;	// Required
		Util::count_t level,split;							// Default
		char future[1024 - 5 * c_t] = {0};					// 0 out
		
		LinearHash_t(
			Util::count_t n , Util::count_t b , Util::count_t e 
		) : numBuckets(n) , bucketSize(b) , elementSize(e) , level(0) , split(0) {
			// No body
		}
		
		LinearHash_t( 
			Util::count_t n , Util::count_t b , Util::count_t e , Util::count_t l , Util::count_t s )
		) : numBuckets(n) , bucketSize(b) , elementSize(e) , level(l) , split(s) {
			// No body
		}
		
	};
		
	
	// Given a file we create the file with the given parameters
	LinearHash &LinearHash::build( const std::string &filename , Util::count_t N , Util::count_t K , Util::count_t ElementSize ) {
		std::filebuf fbuf;
		
		// Create the file
		fbuf.open(filename, std::ios_base::in | std::ios_base::out 
                           | std::ios_base::trunc | std::ios_base::binary); 
		
		// Calculate initial file-size
		Util::count_t elementSize = 2 * c_t + sizeof(Element_t*) + ElementSize;
		Util::count_t bucketSize = 2 * c_t + elementSize;
		Util::count_t headerSize = sizeof(LinearHash_t);
		Util::count_t totalSize = headerSize + N * bucketSize;
		
		
		// Allocate the space
		fbuf.pubseekoff(totalSize, std::ios_base::beg);
		fbuf.sputc(0);
		fbuf.close();
		
		// Create the memory map
		file_mapping m_file(filename, read_write);

		mapped_region region(
			m_file                    	//What to map
			,read_write 				//Map it as read-write
         );
		
		char *mem = (char *)region.get_address();
		
		// Zero out the file
		memset( mem + headerSize , 0 , totalSize );
		
		// Create and write the header
		LinearHash_t header( N , K , ElementSize );
		strncpy( mem , &header , headerSize );
		
		
		return new LinearHash( mem );
	}
	
	LinearHash::LinearHash( char * data , Util::count_t totalSize ) : data_(data) , size_(totalSize) {
		// No body
	}
	
	LinearHash::~LinearHash() {
		munmap( mem , totalSize );
	}
	
	void appendBucket( char *mem , Util::count_t N ) {
		mem = mem + sizeof(LinearHash_t); // Skip header
		
	}
	
	// 
	//
	//
	void LinearHash::put( Util::count_t key , const char* data ) {
		// calculate the position
		Util::count_t pos = key % (N << L);
		if( pos < S ) pos = key % (N << (1 + L));
		
		
		// Check to see if the bucket is full
		
		// if full we 
		if( full ) {
			rehash( this->data_ , pos );
			appendBucket( this->data_ , this->num_buckets_ );
			S = (S==(N << L)) ? 0 : S + 1;
		}
	}
	
	Util::count_t LinearHash::bucketCount() const {
		return this->num_buckets_;
	}
	
	Util::count_t LinearHash::bucketSize() const {
		return this->bucket_size_;
	}
	
	Util::count_t LinearHash::elementSize() const {
		return this->element_size_;
	}
	
	Util::count_t LinearHash::count() const {
		return this->num_elements_;
	}
	

}