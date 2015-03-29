
#ifndef OS_FILESYSTEM_H_

#define OS_FILESYSTEM_H_

#include "Constants.h"
#include "File.h"

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

    enum BlockStatus { FULL , LAZY };

    static const uint64_t TotalBlockSize = KB;
    static const uint64_t HeaderSize = TotalBlockSize;
    static const uint64_t LengthOffset = sizeof(uint64_t) * 10;
    static const uint64_t BlockSize = TotalBlockSize - 3 * sizeof(uint64_t);

    struct Block {
        BlockStatus status;
        uint64_t block,prev,next,length;
        char data[BlockSize];
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

        // Pointers to open files
        std::list<File> openFiles;
        std::list<File> allFiles;

        void lock( LockType );
        void unlock( LockType );

        void split( Block& , uint64_t );
        void flush( Block& );
        Block grow( uint64_t , char*  );
        Block allocate( uint64_t , char* );
        Block lazyLoad( uint64_t );
        Block load( uint64_t );
        Block locate( uint64_t , uint64_t& );
        Block reuse( uint64_t& , char*& );

        uint64_t read( uint64_t start , uint64_t offset , uint64_t length , char *buffer ); 
        uint64_t write( uint64_t start , uint64_t offset , uint64_t length , char *buffer ); 
        uint64_t insert( uint64_t start , uint64_t offset , uint64_t length , char *buffer ); 
        uint64_t remove( uint64_t start , uint64_t offset , uint64_t length ); 

        bool unlink( File );
        bool rename( File , const std::string );

        public:

        FileSystem( const std::string location );
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

        friend File;

    };
}

#endif
