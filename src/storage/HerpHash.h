

#ifndef HERPHASH_H_
#define HERPHASH_H_

#include <functional>
#include <map>
#include <vector>
#include "../utils/Util.h"

namespace Storage {
    template <class KEY, class VALUE,uint64_t Buckets = 1024>
    class HerpHash {
        public:

        class HerpIterator;

        std::vector< std::map<KEY,VALUE> > maps;
        std::hash<KEY> hash_fn;

        uint64_t numBuckets(){
            return Buckets;
        }

        std::map<KEY,VALUE>& Which(KEY &k) {
            return maps[hash_fn(k) % Buckets];
        }
        
        void save( std::map<KEY,VALUE>& m , KEY &k ) {
            maps[hash_fn(k) % Buckets] = m;
        }

        HerpHash() {
            for( int i = 0 ; i < Buckets ; ++i ) {
                maps.push_back( std::map<KEY,VALUE>() );
            }
        }

        HerpHash(const HerpHash& other) {
            for( int i = 0 ; i < Buckets; ++i) {
                auto m = other.maps[i];
                maps[i] = m;
            }
        }

        ~HerpHash() {
        }

        void put( KEY k , VALUE v ) {
            auto m = Which(k);
            m[k] = v;
            save( m , k );
        }

        VALUE get( KEY k) {
            auto m = Which(k);
            return m[k];
        }

        bool contains( KEY k ) {
            auto m = Which(k);
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
                typedef typename std::vector<std::map<KEY,VALUE> >::iterator ITER;
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
                        m_curr = c.begin();
                        m_end = c.end();
                    }
                }

                HerpIterator( ITER curr, ITER end ) : curr(curr) , end(end) {
                    if( curr == end ) return;
                    auto c = *curr;
                    m_curr = c.begin();
                    m_end = c.end();
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

                bool operator==(const HerpIterator& rhs) {
                    if( curr != rhs.curr ) return false;    // Not at some position
                    if( curr == end ) return true;         // We are at the end, don't check iter
                    return m_curr == rhs.m_curr;
                }

                bool operator!=(const HerpIterator& rhs) {
                    return !(operator==(rhs));
                }

                std::pair<const KEY,VALUE>& operator*() {
                    return *m_curr;
                }

        };

    };
};

#endif
