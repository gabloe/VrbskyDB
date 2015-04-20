#include "../parsing/Parser.h"
#include "../storage/LinearHash.h"
#include <rapidjson/document.h>
#include "../mmap_filesystem/Filesystem.h"

#define LENGTH(A) sizeof(A)/sizeof(A[0])
#define UNUSED(id)

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

void execute(Parsing::Query &, Storage::LinearHash<std::string> &);

typedef Storage::LinearHash<uint64_t> INDICES;
//typedef Storage::LinearHash<std::string> META;
typedef std::map<std::string,std::string> META;
typedef Storage::Filesystem FILESYSTEM;

std::string SpecialValueComparisons[] = { "#gt", "#lt", "#eq", "#contains", "#starts", "#ends" };
std::string SpecialKeyComparisons[] = { "#exists", "#isnull", "#isstr", "#isnum", "#isbool", "#isarray", "#isobj" };
