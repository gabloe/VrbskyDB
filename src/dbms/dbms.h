#include "../parsing/Parser.h"
#include "../storage/LinearHash.h"
#include <rapidjson/document.h>

#define LENGTH(A) sizeof(A)/sizeof(A[0])
#define UNUSED(id)

void execute(Parsing::Query &, Storage::LinearHash<std::string> &);
std::string toPrettyString(std::string *);
std::string toPrettyString(rapidjson::Document *);


