#ifndef _HASH_H_
#define _HASH_H_

#ifdef _MSC_VER

// Hash function
uint64_t hash3(const void * key, int len, unsigned int seed = 0) {
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

uint64_t hash2(const char* data, size_t len) {
    uint64_t hash = 0;
    const uint64_t mul = 101;
    for (uint64_t i = 0; i < len; ++i) {
        uint64_t c = data[i];
        hash = mul * hash + c;
    }
    return hash;
}

uint64_t hash1(const char* data, size_t length) {
    uint64_t result = static_cast<uint64_t>(14695981039346656037ULL);
    for (; length; --length) {
        result ^= static_cast<uint64_t>(*data++);
        result *= static_cast<uint64_t>(1099511628211ULL);
    }
    return result;
}

template<typename T>
inline uint64_t hash(T &str, size_t len) {
    //return hash1( str.c_str() , len );
    //return hash2( str.c_str() , len );
    return hash3(str.c_str(), len);		// Murmer
}

#endif

