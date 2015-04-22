extern "C"
{
#ifdef _WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

#define UUID_ALT

#ifdef UUID_ALT
std::string newUUID() {
	static uint64_t i;
	return std::to_string(i++);
}
#else
std::string newUUID()
{
std::string res;
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
    return std::string(s);
}
#endif
