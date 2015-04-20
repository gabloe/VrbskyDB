
#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>

namespace Storage {

    static void Write64( char *buffer , uint64_t &pos , uint64_t value ) {
        memcpy( buffer + pos , &value , sizeof(uint64_t) );
        pos += sizeof(uint64_t);
    }

    static void WriteString( char *buffer , uint64_t &pos , std::string& value ) {
        memcpy( buffer + pos , value.c_str() , value.size() );
        pos += value.size();
    }

    static void WriteRaw( char *buffer , uint64_t &pos , const char* value , uint64_t length ) {
        memcpy( buffer + pos , value , length );
        pos += length;
    }

    static uint64_t Read64( char *buffer , uint64_t &pos ) {
        uint64_t result = *reinterpret_cast<uint64_t*>(buffer + pos);
        pos += sizeof(uint64_t);
        return result;
    }

    static std::string ReadString( char * buffer , uint64_t &pos , uint64_t len ) {
        std::string str( buffer + pos , len );
        pos += len;
        return str;
    }

    template <class K>
        struct Type {
            static uint64_t Size( K ) {
                return sizeof(K);
            }
            static K Create( char* data , uint64_t  ) {
                return *reinterpret_cast<K*>( data );
            }
            static const char* Bytes( K &k ) {
                return reinterpret_cast<char*>(&k);
            }
        };
    template <>
        struct Type<std::string> {
            static uint64_t Size( std::string &msg ) {
                return msg.size();
            }
            static std::string Create(char * data, uint64_t len ) {
                return std::string( data , len );
            }
            static const char* Bytes( std::string &msg ) {
                return msg.c_str();
            }
        };

}

#endif
