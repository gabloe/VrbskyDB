
#include "LinearHash.h"

int main( void ) {
	
	// 100 buckets, 10 elements per bucket, each element holds 512 bytes
	Storage::LinearHash hash = new Storage::LinearHash::build( "test.dat" , 100 , 10 , 512 );
	
	hash.put( 10 , "Hello World" );
	hash.put( 32 , "This sucks" );
	hash.put( 1 ,  "No way" );
	
	hash.flush();
	hash.close();
	
	Storage::LinearHash hash = new Storage::LinearHash::load( "test.dat" );
	
	std::cout << "Number of buckets: " << hash.bucketCount() << std::endl;
	std::cout << "Number of elements per bucket: " << hash.bucketSize() << std::endl;
	std::cout << "Size of an element: " << hash.elementSize() << std::endl;
	std::cout << "Number of stored elements: " << hash.count() << std::endl;
	
	const char* t = hash.get(10);	
	std::count << "hash[10] = " << t << std::endl;
	const char* t = hash.get(32);	
	std::count << "hash[32] = " << t << std::endl;
	const char* t = hash.get(1);	
	std::count << "hash[1] = " << t << std::endl;
	
	if( hash.get( 2 ) != NULL ) {
		std::cout << "Retrieved an element not in the list" << std::endl;
		return -1;
	}else {
		std::cout << "hash[2] = NULL" << std::endl;
	}
	
	return 0;
}