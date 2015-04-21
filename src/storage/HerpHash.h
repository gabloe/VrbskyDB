

#ifndef HERPHASH_H_
#define HERPHASH_H_

#include <functional>
#include <map>
//#include <vector>
#include <array>
#include "../utils/Util.h"

namespace Storage {
    template <typename KEY, typename VALUE, uint64_t Buckets = 1024>
    class HerpHash {
        public:

        class HerpIterator;

        // Vector of pointers to std::maps
        std::array< std::map<KEY,VALUE>* , Buckets> maps;
        std::hash<KEY> hash_fn;

        std::map<KEY,VALUE>& Which(KEY &k) {
            return *maps[hash_fn(k) % Buckets];
        }
        
        void save( std::map<KEY,VALUE>& m , KEY &k ) {
            maps[hash_fn(k) % Buckets] = m;
        }

        HerpHash() {
            for( int i = 0 ; i < Buckets ; ++i ) {
                maps[i] = new std::map<KEY,VALUE>();
            }
        }

        HerpHash(const HerpHash& other) {
            for( int i = 0 ; i < maps.size(); ++i) {
                auto o = other.maps[i];
                auto m = new std::map<KEY,VALUE>(o->begin(), o->end());
                this->maps[i] = m;
            }
        }

        ~HerpHash() {
            for( int i = 0 ; i < maps.size() ; ++i ) {
                delete maps[i];
            }
        }

        HerpHash& operator=( const HerpHash& rhs) {
            for( int i = 0 ; i < maps.size(); ++i) {
                delete maps[i];
                auto o = rhs.maps[i];
                auto m = new std::map<KEY,VALUE>(o->begin(), o->end());
                this->maps[i] = m;
            }
            return *this;
        }

        VALUE&  operator[]( KEY &k ) {
            return Which(k)[k];
        }

        size_t count( KEY &k ){
            return contains( k );
        }

        void erase( KEY &k ) {
            for( int i = 0 ; i < Buckets ; ++i ) {
                maps[i]->erase( k );
            }
        }

        size_t size() {
            int s = 0;
            for( int i = 0 ; i <Buckets; ++i ) {
                s += maps[i]->size();
            }
            return s;
        }

        void put( KEY k , VALUE v ) {
            std::map<KEY,VALUE>& m = Which(k);
            m[k] = v;
        }

        VALUE get( KEY k) {
            std::map<KEY,VALUE>& m = Which(k);
            return m[k];
        }

        bool contains( KEY k ) {
            std::map<KEY,VALUE>& m = Which(k);
            return m.count(k) > 0;
        }

        HerpIterator begin() {
            return HerpIterator( maps.begin() , maps.end() );
        }

        HerpIterator end() {
            return HerpIterator( maps.end() , maps.end() );
        }

        class HerpIterator : public std::iterator<std::input_iterator_tag,std::pair<KEY,VALUE> > {
            public:
                //typedef typename std::vector<std::map<KEY,VALUE>* >::iterator ITER;
                typedef typename std::array<std::map<KEY,VALUE>* , Buckets>::iterator ITER;
                typedef typename std::map<KEY,VALUE>::iterator IN;

                ITER curr,end;
                IN m_curr, m_end;

                void m_next() {
                    if( curr == end ) return;

                    while( m_curr == m_end ) {
                        ++curr;
                        if( curr == end ) {
                            break;
                        }
                        auto c = *curr;
                        m_curr = c->begin();
                        m_end = c->end();
                    }
                }

                HerpIterator( ITER curr, ITER end ) : curr(curr) , end(end) {
                    if( curr == end ) return;
                    auto c = *curr;
                    m_curr = c->begin();
                    m_end = c->end();
                    m_next();
                }

                HerpIterator( const HerpIterator& other) : 
                    curr(other.curr), end(other.end),
                    m_curr(other.m_curr), m_end(other.m_end) {}

                HerpIterator& operator++() {
                    // Done, just leave
                    if( curr != end ) { 
                        ++m_curr;
                        if( m_curr == m_end ) m_next();
                    }

                    return *this;
                }

                HerpIterator& operator++(int) {
                    return operator++();
                }

                std::pair<const KEY,VALUE>* operator->() const {
                    return &(*m_curr);
                }

                bool operator==(const HerpIterator& rhs) {
                    if( curr != rhs.curr ) return false;    // Not at some position
                    if( curr == end ) return true;         // We are at the end, don't check iter
                    return m_curr == rhs.m_curr;
                }

                bool operator!=(const HerpIterator& rhs) {
                    return !(operator==(rhs));
                }

                std::pair<const KEY,VALUE>* operator*() {
                    return m_curr;
                }

                KEY first() {
                    return m_curr->first;
                }

                VALUE second() {
                    return m_curr->second;
                }

        };

    };
};

#endif
