
#ifndef OS_FILEREADER_H_
#define OS_FILEREADER_H_

#include "Constants.h"
#include "File.h"

namespace os {

    class FileReader {
        private:
            File file;
        public:

        FileReader(File f);
        uint64_t read( uint64_t , char* );
        void seek( uint64_t , FilePosition );
        void close();

    };
};

#endif
