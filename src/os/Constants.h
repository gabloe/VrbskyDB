
#ifndef OS_CONSTANTS_H_
#define OS_CONSTANTS_H_

#include <cstdint>

namespace os {

    enum FileStatus { OPEN , CLOSED , DELETED };
    enum LockType { READ , WRITE };

    static const uint64_t KB = 1024;
    static const uint64_t MB = 1024 * KB;
    static const uint64_t GB = 1024 * MB;
    static const uint64_t TB = 1024 * GB;

}

#endif
