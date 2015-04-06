
#include "../os/File.h"
#include "../os/FileSystem.h"

namespace os {

    FileStatus File::getStatus() const {
        return status;
    }

    bool File::unlink() {
        return fs->unlink( *this );
    }

    bool File::rename( const std::string newName ) {
        return fs->rename( *this , newName );
    }

}
