
#ifndef OS_FILEREADER_H_
#define OS_FILEREADER_H_

#include "Constants.h"
#include "File.h"
#include <assert.h>

namespace os {

    class FileReader {
        friend class FileSystem;

        private:
            File& file;
        public:

        FileReader(File& f);
        std::string readAll();
        uint64_t read( uint64_t , char* );
        void seek( uint64_t , FilePosition );
        uint64_t tell();
        void close();

    };
};

#endif
