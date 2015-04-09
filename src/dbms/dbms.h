#include "../parsing/Parser.h"
#include "../storage/LinearHash.h"
#include <rapidjson/document.h>
#include "../os/FileSystem.h"

#define LENGTH(A) sizeof(A)/sizeof(A[0])
#define UNUSED(id)

void execute(Parsing::Query &, Storage::LinearHash<std::string> &);

typedef Storage::LinearHash<uint64_t> INDICES;
typedef Storage::LinearHash<std::string> META;
typedef os::FileSystem FILESYSTEM;

std::string SpecialValueComparisons[] = { "#gt", "#lt", "#eq", "#contains", "#starts", "#ends" };
std::string SpecialKeyComparisons[] = { "#exists", "#isnull", "#isstr", "#isnum", "#isbool", "#isarray", "#isobj" };
