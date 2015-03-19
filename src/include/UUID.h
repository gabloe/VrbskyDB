extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

unsigned long long newUUID64();
unsigned int newUUID();

uint64_t newUUID64()
{
   return ((uint64_t)newUUID() << 32) | (uint64_t)newUUID();
}

unsigned int newUUID()
{
std::string res("0x");
#ifdef WIN32
    UUID uuid;
    UuidCreate ( &uuid );

    unsigned char * str;
    UuidToStringA ( &uuid, &str );

    std::string s( ( char* ) str );

    RpcStringFreeA ( &str );
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
#endif
    res += s;
    return std::stoul(res, nullptr, 16);
}
