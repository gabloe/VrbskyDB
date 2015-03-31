
#include "File.h"

#include "FileSystem.h"

namespace os {

    uint64_t File::read( uint64_t length , char *buffer ) {
        uint64_t offset = position;
        position += length;
        return fs->read( block , offset , length , buffer );
    }

    uint64_t File::write( uint64_t length , const char *buffer ) {
        uint64_t offset = position;
        position += length;
        return fs->write( block , offset , length , buffer );
    }

    uint64_t File::insert( uint64_t length , const char *buffer ) {
        uint64_t offset = position;
        position += length;
        return fs->insert( block , offset , length , buffer );
    }

    uint64_t File::remove( uint64_t length) {
        return fs->remove( block , position , length );
    }


    uint64_t File::read( uint64_t offset , uint64_t length , char *buffer ) {
        return fs->read( first , offset , length , buffer );
    } 

    uint64_t File::write( uint64_t offset , uint64_t length , const char *buffer ) {
        return fs->write( first , offset , length , buffer );
    } 

    uint64_t File::insert( uint64_t offset , uint64_t length , const char *buffer ) {
        return fs->insert( first , offset , length , buffer );
    } 

    uint64_t File::remove( uint64_t offset , uint64_t length ) {
        return fs->remove( first , offset , length );
    } 

    FileStatus File::getStatus() const {
        return status;
    }

    bool File::unlink() {
        return fs->unlink( *this );
    }

    bool File::rename( const std::string newName ) {
        return fs->rename( *this , newName );
    }

    void File::close() {
        status = CLOSED;
        fs->closing( this );
    }

}
