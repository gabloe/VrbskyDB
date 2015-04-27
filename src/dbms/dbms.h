#ifndef DBMS_H_
#define DBMS_H_

#include <list>

#include "../mmap_filesystem/Filesystem.h"
#include "../storage/HerpHash.h"

#define LENGTH(A) sizeof(A)/sizeof(A[0])
#ifndef UNUSED
#define UNUSED(id)
#endif

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

typedef std::list<std::string> DOCDS;

const uint64_t Num_Buckets = 2048;

typedef Storage::HerpHash<std::string,DOCDS, Num_Buckets> META;
typedef Storage::Filesystem FILESYSTEM;

std::string SpecialValueComparisons[] = { "#gt", "#lt", "#eq", "#contains", "#starts", "#ends" };
std::string SpecialKeyComparisons[] = { "#exists", "#isnull", "#isstr", "#isnum", "#isbool", "#isarray", "#isobj" };

#endif
