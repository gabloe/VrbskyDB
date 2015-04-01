
#ifndef OS_FILESYSTEM_H_

#define OS_FILESYSTEM_H_

#include "Constants.h"

#include <ios>
#include <fstream>
#include <string>
#include <list>


// FileStructure
// Header
//
//  The header consists of data which is required for the FileSystem class to work.
//  The structure is as such
//  struct Header {
//      char magic[] = { 0xD , 0xE , 0xA , 0xD , 0xB , 0xE , 0xE , 0xF };
//      uint64_t numBlocks;
//      uint64_t numFreeBlocks;
//      uint64_t firstFreeBlock;
//      uint64_t numFiles;
//  };
//  
//
// Filenames
// Data

namespace os {

    class File;

    enum BlockStatus { FULL , LAZY };

    static const uint64_t SignatureSize = 8;
    static const char HeaderSignature[SignatureSize] = { 0xD , 0xE , 0xA , 0xD , 0xB , 0xE , 0xE , 0xF  };

    static const uint64_t TotalBlockSize = KB;
    static const uint64_t HeaderSize = TotalBlockSize;
    static const uint64_t LengthOffset = sizeof(uint64_t) * 10;
    static const uint64_t BlockSize = TotalBlockSize - 3 * sizeof(uint64_t);

    struct Block {
        BlockStatus status;
        uint64_t block,prev,next,length;
        char data[BlockSize];
        Block() {
            status = LAZY;
            block = prev = next = length = 0;
        }
    };

    class FileSystem {

        private:

            std::fstream stream;

            std::string fileSystemLocation;
            uint64_t totalBytes;
            uint64_t freeList;
            uint64_t numBlocks;
            uint64_t numFreeBlocks;
            uint64_t numFiles;
            uint64_t metadataSize;

            // Pointers to open files
            std::list<File> openFiles;
            std::list<File> allFiles;

            void lock( LockType );
            void unlock( LockType );


            /* File management functions */
            File createNewFile( std::string );

            /* Data management functions*/
            void saveHeader();
            void split( Block& , uint64_t );
            void flush( Block& );
            Block grow( uint64_t , const char*  );
            Block allocate( uint64_t , const char* );
            Block lazyLoad( uint64_t );
            Block load( uint64_t );
            Block locate( uint64_t , uint64_t& );
            Block reuse( uint64_t& , const char*& );

            void closing( File* );

            uint64_t read( File &f , uint64_t length , char *buffer ); 
            uint64_t write( File &f , uint64_t length , const char *buffer ); 
            uint64_t insert( File &f , uint64_t length , const char *buffer ); 
            uint64_t remove( File &f, uint64_t length ); 

            bool unlink( File );
            bool rename( File& , const std::string );

            FileSystem() = delete;

        public:

            FileSystem( const std::string location );
            FileSystem( const FileSystem &other ) = delete;
            FileSystem( FileSystem &other ) = delete;
            ~FileSystem();

            bool moveFileSystem( const std::string newLocation );

            std::list<File*> getFiles();
            std::list<File*> getOpenFiles();
            std::list<std::string> getFilenames();

            // Open a file for reading or modifiying.  If the file does not exist it is created.
            File open( const std::string );

            // Checks to see if a file exists
            bool exists( const std::string ); 

            // Delete a file by name
            bool unlink( const std::string );

            bool rename( const std::string oldName , const std::string newName );

            void shutdown();

            friend class File;
            friend class FileReader;
            friend class FileWriter;

    };
}

#endif
