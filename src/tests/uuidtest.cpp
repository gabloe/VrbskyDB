#include <iostream>
#include <string>
#include <UUID.h>

int main(void) {
	uint64_t *arr = new uint64_t[10000];
	std::cout << "Creating 10000 UUID's and testing for collisions." << std::endl;
	for (size_t i=0; i<10000; i++) {
		arr[i] = newUUID64();
	}
	for (size_t i=0; i<10000; i++) {
		for (size_t j=0; j<10000; j++) {
			if (j==i) continue;
			if (arr[i]==arr[j]) {
				std::cout << "Collision occurred! " << arr[i] << std::endl;
				delete arr;
				return 0;
			}
		}
	}
	delete arr;
        std::cout << "Passed!" << std::endl;
	return 0;
}
