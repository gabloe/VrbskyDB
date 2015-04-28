
#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <algorithm>
#include <list>
#include <vector>
#include <string>


    static void Write64( char *buffer , uint64_t &pos , uint64_t value ) {
        char *herp = reinterpret_cast<char*>(&value);
        std::copy( herp , herp + sizeof(uint64_t) , buffer + pos );
        pos += sizeof(uint64_t);
    }

    static void WriteString( char *buffer , uint64_t &pos , std::string& value ) {
        std::copy( value.begin() , value.end() , buffer + pos );
        pos += value.size();
    }

    static void WriteRaw( char *buffer , uint64_t &pos , const char* value , uint64_t length ) {
        std::copy( value , value + length , buffer + pos );
        pos += length;
    }

    static uint64_t Read64( const char *buffer , uint64_t &pos ) {
        uint64_t result = *reinterpret_cast<const uint64_t*>(buffer + pos);
        pos += sizeof(uint64_t);
        return result;
    }

    static std::string ReadString( const char * buffer , uint64_t &pos , uint64_t len ) {
        std::string str( buffer + pos , len );
        pos += len;
        return str;
    }

    template <class K>
        struct Type {
            static uint64_t Size( K ) {
                return sizeof(K);
            }
            static K Create( const char* data , uint64_t  ) {
                return *reinterpret_cast<const K*>( data );
            }
            static const char* Bytes( K &k ) {
                uint64_t size = sizeof(K);
                char *buff = new char[size];
                char *data = reinterpret_cast<char*>(&k);
                std::copy( data , data + size , buff );
                return buff;
            }
            static const std::string Name() {
                return std::string( typeid(K).name() );
            }
        };
    template <>
        struct Type<std::string> {
            static uint64_t Size( std::string &msg ) {
                return msg.size();
            }
            static std::string Create(const char * data, uint64_t len ) {
                return std::string( data , len );
            }
            static const char* Bytes( std::string &msg ) {
                char *buff = new char[msg.size()];
                return buff;
            }
            static const std::string Name() {
                return "std::string";
            }
        };

    template <>
        struct Type<std::list<std::string> > {
            static uint64_t Size( std::list< std::string > &msg ) {
                uint64_t sum = 0;
                for( auto herp = msg.begin() ; herp != msg.end() ; ++herp ) {
                    sum += (*herp).size() + sizeof(uint64_t);
                }
                return sum;
            }
            static std::list<std::string> Create(const char * data, uint64_t len) {
                std::list< std::string > ret;
                uint64_t pos = 0;
                while( pos < len ) {
                    uint64_t l = Read64( data , pos );
                    ret.push_back( ReadString( data , pos , l ) );
                }
                return ret;
            }
            static const char* Bytes( std::list<std::string> &er ) {
                int length = Type::Size(er);
                char *buff = new char[length];
                uint64_t pos = 0;
                for( auto i = er.begin() ; i != er.end() ; ++i ) {
                    std::string& s = *i;
                    Write64( buff , pos , s.size() );
                    WriteString( buff , pos , s );
                }
                return buff;
            }
            static const std::string Name() {
                return "std::list<std::string>";
            }
        };

    template <>
        struct Type<std::vector<std::string> > {
            static uint64_t Size( std::vector< std::string > &msg ) {
                uint64_t sum = 0;
                for( auto herp = msg.begin() ; herp != msg.end() ; ++herp ) {
                    sum += (*herp).size() + sizeof(uint64_t);
                }
                return sum;
            }
            static std::vector<std::string> Create(const char * data, uint64_t len) {
                std::vector< std::string > ret;
                uint64_t pos = 0;
                while( pos < len ) {
                    uint64_t l = Read64( data , pos );
                    ret.push_back( ReadString( data , pos , l ) );
                }
                return ret;
            }
            static const char* Bytes( std::vector<std::string> &er ) {
                int length = Type::Size(er);
                char *buff = new char[length];
                uint64_t pos = 0;
                for( auto i = er.begin() ; i != er.end() ; ++i ) {
                    std::string& s = *i;
                    Write64( buff , pos , s.size() );
                    WriteString( buff , pos , s );
                }
                return buff;
            }
            static const std::string Name() {
                return "std::vector<std::string>";
            }
        };


#endif
