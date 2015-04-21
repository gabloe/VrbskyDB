
#include <string>

#include "../storage/HerpHash.h"
#include "../mmap_filesystem/HerpmapWriter.h"
#include "../mmap_filesystem/HerpmapReader.h"

void write( std::string prefix , int limit ) {
	Storage::Filesystem fs("data.db");
    File f = fs.open_file( "merp" );

    Storage::HerpHash<std::string,int> merp;
    for( int value = 0 ; value < limit ; ++value ) {
        std::string key( prefix + std::to_string( value ) );
        merp.put( key , value );
    }
    Storage::HerpmapWriter<int> writer( f , &fs );
    writer.write( merp );
    std::cout << "File has size " << f.size << std::endl;
    fs.shutdown();
}

void read( std::string prefix , int limit ) {
	Storage::Filesystem fs("data.db");
    File f = fs.open_file( "merp" );
    std::cout << "File has size " << f.size << std::endl;
    Storage::HerpmapReader<int> reader( f , &fs );

    Storage::HerpHash<std::string,int> merp = reader.read(); ;

    int seen = 0;

    for( int value = 0 ; value < limit ; ++value ) {
        std::string key( prefix + std::to_string( value ) );
        Assert( "Missing" , merp.contains( key ) );
        int test = merp.get( key );
        Assert( "Returned value does not go with key" , key , test , test == value );
        ++seen;
    }
    Assert( "Number seen does not match expected" , seen , limit , seen == limit );
    fs.shutdown();
}

int main(int argc, char *argv[]) {
    int limit = 10;
    if( argc == 2 ) {
        limit = atoi( argv[1] );
    }
    limit = std::max( limit , 10 );
    write( "herp" , limit );
    read( "herp" , limit );
    return 0;
}
