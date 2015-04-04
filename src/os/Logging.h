
#include <ostream>

class Log {

    private:
        std::ostream &output;
        int depth = 0;
        bool enabled = false;

        void tabs() {
            for( int i = 0 ; i < depth ; ++i ) output << "\t";
        }
    public:
        Log( std::ostream& out ) : output(out) {
            /* Empty */
        }

        void enable() {
            enabled = true;
        }

        void disable() {
            enabled = false;
        }

        template<class T>
        void log( T t , bool force = false ) {
            if( enabled || force ) {
                tabs();
                output << "Log: " << t << std::endl;
            }
        }

        template<class K,class V>
        void log( K k , V v , bool force = false ) {
            if( enabled || force ) {
                tabs();
                output << k << ": " << v << std::endl;
            }
        }

        void enter( std::string method ) {
            log( method , true );
            ++depth;
        }

        void leave( std::string method ) {
            --depth;
            log( method , true );
        }

};

