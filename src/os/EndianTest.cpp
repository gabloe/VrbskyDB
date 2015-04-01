#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE
#else
#define BIG
#endif

#include <iostream>

int main(void) {
#ifdef BIG
std::cout << "Big Endian" << std::endl;
#else
std::cout << "Little Endian" << std::endl;
#endif	
	return 0;
}
