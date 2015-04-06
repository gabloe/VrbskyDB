
#include <ostream>

class Log {

    private:
        std::ostream &output;
        int depth = 0;
        bool enabled = true;
        bool methodEnabled = true;
        bool working = true;

        void tabs() {
            for( int i = 0 ; i < depth ; ++i ) output << "\t";
        }
    public:
        Log( std::ostream& out ) : output(out) {
            /* Empty */
        }

        void toggleOutput() {
            working = !working;
        }

        void enable() {
            enabled = true;
        }

        void enableMethod() {
            methodEnabled = true;
        }

        void disable() {
            enabled = false;
        }

        void disableMethod() {
            methodEnabled = false;
        }

        template<class T>
            void log( T t , bool force = false ) {
                if( working ) {
                    if( enabled || force ) {
                        tabs();
                        output << "Log: " << t << std::endl;
                    }
                }
            }

        template<class K,class V>
            void log( K k , V v , bool force = false ) {
                if( working ) {
                    if( enabled || force ) {
                        tabs();
                        output << k << ": " << v << std::endl;
                    }
                }
            }

        void enter( std::string method ) {
            if( methodEnabled ) {
                log( method , true );
                ++depth;
            }
        }

        void leave( std::string method ) {
            if( methodEnabled ) {
                --depth;
                log( method , true );
            }
        }

};

