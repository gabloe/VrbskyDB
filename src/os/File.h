
#ifndef OS_FILE_H_
#define OS_FILE_H_


namespace os {

        class File {

            private:

                // Properties of file/data
                std::string name;
                FileStatus status;
                uint64_t size;

                // Position information
                uint64_t first;    // First block
                uint64_t last;    // First block
                uint64_t block;     // Current block
                uint64_t position;  // Current position in the block

                File() {} 
                File( const File &other ) {
                    name = other.name;
                    status = other.status;
                    size = other.size;

                    first = other.first;
                    last = other.last;
                    block = other.block;
                    position = other.position;
                }

            public:

                std::string getFilename() const {
                    return name;
                }

                int read( uint64_t length , char* buffer ) const ;
                int write( uint64_t length , char* buffer );
                int append( uint64_t length , char* buffer );
                int remove( uint64_t length );

                int read( uint64_t start , uint64_t length , char* buffer ) const;
                int write( uint64_t start , uint64_t length , char* buffer );
                int append( uint64_t start , uint64_t length , char* buffer );
                int remove( uint64_t start , uint64_t length );

                bool unlink();
                bool rename( const std::string newName );
                FileStatus getStatus() const;

                friend class FileSystem;
        };
}


#endif
