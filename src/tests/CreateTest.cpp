#include <immap_filesystemtream>

#include "../mmap_filesystem/FileSystem.h"
#include "../mmap_filesystem/File.h"
#include "../mmap_filesystem/FileWriter.h"

int main( void ) {
    const std::string FileName = "test.dat";
    Storage::FileSystem fs( FileName );
    fs.shutdown();

    return 0;
}
