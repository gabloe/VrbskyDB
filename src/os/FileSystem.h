
#ifndef OS_FILESYSTEM_H_

#define OS_FILESYSTEM_H_

#include <iostream>

#include <ios>
#include <fstream>
#include <string>
#include <list>
#include <array>

#include "../os/Constants.h"


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
    class FileWriter;
    class FileReader;

    enum BlockStatus { FULL , LAZY };

    static const uint64_t SignatureSize = 8;
    static const std::array<char,8> HeaderSignature = { {0xD , 0xE , 0xA , 0xD , 0xB , 0xE , 0xE , 0xF}  };

    static const uint64_t Total_Size_Block = 128 * B;
    static const uint64_t Header_Size = Total_Size_Block;
    static const uint64_t Block_Size = Total_Size_Block - 3 * sizeof(uint64_t);

    struct Block {
        BlockStatus status;
        uint64_t block,prev,next,length;
        std::array<char,Block_Size> data;
        Block() {
            status = LAZY;
            block = prev = next = length = 0;
        }
    };

    class FileSystem {

        private:

            std::fstream stream;

            std::string fileSystemLocation;
            uint64_t bytes_allocated             = Block_Size;
            uint64_t bytes_used                  = 0;
            uint64_t blocks_allocated            = 2;
            uint64_t blocks_used                 = 0;    // Given to file-system
            uint64_t free_count                  = 0;
            uint64_t free_first                  = 0;
            uint64_t metadata_bytes_allocated    = Block_Size;
            uint64_t metadata_allocated_blocks   = 1;
            uint64_t metadata_bytes_used         = 0;
            uint64_t metadata_blocks_used        = 0;
            uint64_t metadata_files              = 0;
            uint64_t metadata_start              = 1;
            uint64_t metadata_end                = 1;

            // Pointers to open files
            std::list<File*> openFiles;
            std::list<File*> allFiles;

            FileWriter *metaWriter;
            FileReader *metaReader;
            File       *metadata;

            void lock( LockType );
            void unlock( LockType );


            /* File management functions */
            void loadMetadata();
            uint64_t insertFile( File &f );
            File& createNewFile( std::string );

            /* Data management functions*/
            void saveHeader();
            void readHeader();
            void printHeader(bool);

            void gotoBlock( uint64_t );
            Block readBlock();
            void writeBlock( Block &b );

            void split( Block& , uint64_t );
            void flush( Block& );
            Block grow( uint64_t , const char *  );
            Block allocate( uint64_t , const char * );
            Block lazyLoad( uint64_t );
            Block load( uint64_t );
            Block locate( uint64_t , uint64_t& );
            Block reuse( uint64_t& , const char*& );

            void closing( File& );

            uint64_t read(      File& f, uint64_t length , char *buffer ); 
            uint64_t write(     File& f, uint64_t length , const char *buffer ); 
            uint64_t insert(    File& f, uint64_t length , const char *buffer ); 
            uint64_t remove(    File& f, uint64_t length ); 

            bool unlink( File& );
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
            File& open( const std::string );
            void close( const std::string );

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
