
#include "LinearHash.h"




namespace Storage {

    const uint64_t KB = 1024;
    const uint64_t MB = 1024 * KB;
    const uint64_t GB = 1024 * MB;

    const uint64_t None         = 0;
    const uint64_t HeaderItems  = 3;
    const uint64_t DocumentSize = KB;
    const uint64_t DataSize     = DocumentSize - sizeof(int) - sizeof(Document*);
    const uint64_t HeaderSize   = HeaderItems * sizeof(uint64_t);

    struct DataBase {
        // Not stored in file
        std::string filename;

        // Stored in file
        uint64_t    num_documents;
        uint64_t    freeList;
        uint64_t    documents;

        // Given a string document return the offset
        uint64_t insert( std::string doc ) {
            size_t remaining = doc.length();
            const char* d = doc.c_str();

            Document d = freeDocument();

            do {
                remaining = d.setData( d , remaining );
            } while ( remaining > 0 );

            return d.offset;
        }

        Document getEmptyDocument() {
            // Create new document
            if( freeList == None ) {
                ++num_documents;
                return Document( db );
            }

            Document d = Document( filename , freeList );
            freeList = d.next;

        }

    };

    DataBase Create(std::string filename , uint64_t number_documents ) {

        std::ofstream dataFile( filename , std::ios::binary );

        if( !dataFile ) {
            throw new Exception( "Could not create new database: " + filename );
        }

        freeList    = HeaderSize;
        documents   = 0;

        // Write the header
        dataFile.write( reinterpret_cast<char*>(&number_documents)  , sizeof(number_documents) ); 
        dataFile.write( reinterpret_cast<char*>(&freeList)          , sizeof(freeList) ); 
        dataFile.write( reinterpret_cast<char*>(&documents)         , sizeof(documents) ); 
        
        // Set the document defaults
        for( int i = 0 ; i < number_documents - 1; ++i ) {
            dataFile.write( None , sizeof(uint64_t) );
            dataFile.write( HeaderSize + DocumentSize * (i+1) , sizeof( uint64_t ) );
            dataFile.seekg( HeaderSize + DocumentSize * (i+1) );
        }

        // Last document in freelist points to nothing
        dataFile.write( None , sizeof(uint64_t) );
        dataFile.write( None , sizeof( uint64_t ) );

        dataFile.close();

        Database db;
        db.filename = filename;
        db.num_documents = number_documents;
        db.freeList = freeList;
        db.documents = documents;

        return db;
    }


    struct Document {
        uint64_t offset;
        uint64_t length;
        uint64_t next;

        char *data;

        // Load a document given a file and offset
        Document( std::string filename , uint64_t offset ) {
            
            // Open the file
            std::ifstream dataFile( filename , std::ios::binary );
            dataFile.rdbuf()->pubsetbuf( std::iobuf , sizeof std::iobuf );
            if (!dataFile) {
                throw new Exception( "File could not be opened to read document: " + filename );
            }


            // Temp buffer/vector
            char buff[DataSize];
            std::vector<char> data;

            uint64_t length,to_read,read = 0;

            do {
                // Go to spot
                dataFile.seek( offset );

                // Read length
                dataFile.read( reinterpret_cast<char*>(&length) , sizeof( length ) );
                to_read = min( DataSize , length );

                // Read data
                dataFile.read( buff , to_read ); 
                data.insert( data.end() , buff , buff + to_read );
                read += to_read;

                // Read next
                dataFile.read( reinterpret_cast<char*>(&offset) , sizeof( offset ) );

            }while( offset != 0 );
            
            // Save data
            this->offset = offset;
            this->length = read;
            this->data = new char[read];
            std::copy( v.being() , v.end() , this->data );

        }

        ~Document() {
            if( data != NULL ) {
                delete[] data;
            }
        }

    }

    struct Loader {

    }
}
