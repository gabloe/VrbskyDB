#include "Filesystem.h"

int main(void) {
	Storage::Filesystem *fs = new Storage::Filesystem("data.db");
	fs->shutdown();
    delete fs;
    return 0;
}
