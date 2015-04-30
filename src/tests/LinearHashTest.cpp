#include "../storage/LinearHash.h"
#include <string>

template <typename T>
void test_insert(Storage::LinearHash<T> &table, T data) {
	int count = 0;
	do {
		++count;
		uint64_t key = hash(data, data.size());

		if (table.contains(key)) {
			std::string table_value;
            table.get(key,table_value);
			uint64_t table_key = hash(table_value, table_value.size());

			std::cout << "Collision at " << count << ": <" << key << "," << data << ">, <" << table_key << ", " << table_value << ">" << std::endl;
			exit(-1);
		}
        std::cout << "About to insert: " << data << std::endl;

		table.put(key, data);
	} while (std::next_permutation(data.begin(), data.end()));
}

template <typename T>
void test_remove(Storage::LinearHash<T> &table, std::string data) {
	int count = 0;
	do {
		++count;
        uint64_t h = hash( data , data.size() );

        std::string data;
        table.get( h , data );

		if ( !table.remove( h ) ) {
			std::cout << "ERROR (remove): " << data << " at " << count << std::endl;
			exit(-1);
		}
	} while (std::next_permutation(data.begin(), data.end()));
}

template <typename T>
void test_contains(Storage::LinearHash<T> &table, std::string data) {
	int count = 0;
	do {
		++count;
        uint64_t h = hash( data , data.size() );
		if ( !table.contains(h) ) {
			std::cout << "ERROR (contains): " << data << " at " << count << std::endl;
			exit(-1);
		}
	} while (std::next_permutation(data.begin(), data.end()));
}


void test(uint64_t buckets, uint64_t elements) {

	std::cout << "Configuration: " << buckets << ":" << elements << std::endl;
	clock_t start, end;
	std::string data("ABCD");

	Storage::LinearHash<std::string> table(buckets, elements);

	// Test inserts
	start = std::clock();
	test_insert<std::string>(table, data);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

	// Test contains/gets
	start = std::clock();
	test_contains(table, data);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to fetch." << std::endl;

	start = std::clock();
	test_remove<std::string>(table, data);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to remove." << std::endl << std::endl;
}

int main(void) {

	if( tests ) {
		for (uint64_t buckets = 2; buckets < 4096; buckets *= 2) {
			for (uint64_t bucket_size = 1; bucket_size < 4096; bucket_size *= 2) {
				test(buckets, bucket_size);
			}
			std::cout << std::endl;
		}
	}
	

	clock_t start, end;

	Storage::LinearHash<std::string> table(1024, 2048);

	std::string data("ABCD");

	// Test inserts
	start = std::clock();
	test_insert(table, data);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to insert." << std::endl;

	// Test contains/gets
	start = std::clock();
	test_contains(table, data);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to fetch." << std::endl;

	// Info
	std::cout << std::endl;
	std::cout << "Hashmap contains " << table.count() << " items" << std::endl;
	std::cout << "Total buckets: " << table.bucket_count() << std::endl;

	// Test write/read from file
	start = std::clock();
	dumpToFile("output.dat", table);
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to write to disk." << std::endl;

	start = std::clock();
	Storage::LinearHash<std::string> *file_table = readFromFile<std::string>("output.dat");
	end = std::clock();
	std::cout << "Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << "ms to read from disk." << std::endl;

	test_contains(*file_table, data);

	delete file_table;
	return 0;
}
