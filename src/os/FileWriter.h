
#ifndef OS_FILEWRITER_H_
#define OS_FILEWRITER_H_

#include "../os/Constants.h"
#include "../os/File.h"
#include "../os/FileWriter.h"

namespace os {

    class FileWriter {

        friend class FileSystem;

        protected:
            File& file;

        public:
            FileWriter( File& f );

            uint64_t write( const uint64_t , const char* const );
            void seek( int64_t , FilePosition );
            uint64_t tell();
            void close();
    };

};

#endif
