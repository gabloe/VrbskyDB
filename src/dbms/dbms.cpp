
#include "../include/config.h"

#include <fstream> 
#include <sstream> 
#include <limits>
#include <cstddef>
#include <time.h>
#include <map>
#include <math.h>
#include <ctime>
#include <cstring>
#include <stdarg.h>

#include "dbms.h"
#include "Aggregator.h"

#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"
#include "../mmap_filesystem/Filesystem.h"

#include "../mmap_filesystem/HerpmapWriter.h"
#include "../mmap_filesystem/HerpmapReader.h"

#if THREADING
#include "../threading/ThreadPool.h"
#endif

#include <pretty.h>
#include <UUID.h>
#include <linenoise/linenoise.h>

#if THREADING
ThreadPool pool(NUM_THREADS);
#endif

std::ostream&
PRINT_ONE(std::ostream& os)
{
    return os;
}

template <class A0, class ...Args>
std::ostream&
PRINT_ONE(std::ostream& os, const A0& a0, const Args& ...args)
{
    os << a0;
    return PRINT_ONE(os, args...);
}

template <class ...Args>
std::ostream&
PRINT(std::ostream& os, const Args& ...args)
{
    return PRINT_ONE(os, args...);
}

template <class ...Args>
std::ostream&
PRINT(const Args& ...args)
{
    static std::mutex m;
    std::lock_guard<std::mutex> _(m);
    return PRINT(std::cout, args...);
}

/*
 *      appendDocToProject ---
 *      
 *      Retrieve the array of documents for the specified project from the metadata.
 *      Insert the new document ID into the array.
 *      Replace the array in the metadata with the modified array.
 *
 */

void appendDocToProject(std::string &project, std::string &doc, META &meta) {
    if (meta.count(project) > 0) {
        meta[project].push_back(doc);
    } else {
        DOCDS data;
        data.push_back(doc);
        meta[project] = data;
    }
}

/*
 *      insertDocument ---
 *      
 *      1.)
 *      Assign the document a UUID
 *      
 *      2.)
 *      Iterate over each field of the document and create a row containing the document ID and a field ID
 *      Insert the row into the database.
 *
 *      TODO: Inserting the data into the DB
 *
 */

void insertDocument(std::string& docUUID, std::string &doc, std::string &project, META &meta, FILESYSTEM &fs) {
    // Insert this row into the DB
    File file = fs.open_file(docUUID.c_str());
    fs.write(&file, doc.c_str(), doc.size());
    appendDocToProject(project, docUUID, meta);
}

/*
 *      updateProjectList ---
 *
 *      Insert the project name into the metadata array of project names.
 *
 */

void updateProjectList(std::string &pname, META &meta) {
    std::string key("__PROJECTS__");

    if (meta.count(key) == 0) {
        DOCDS data;
        data.push_back(pname);
        meta[key] = data;
        return;
    }
    DOCDS& data = meta[key];

    // Check if the project already exists
    bool found = false;
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string &v = *it;
        if (v.compare(pname) == 0) {
            found = true;
            break;
        }
    }

    // Only add the project if it is not found
    if (!found) {
        data.push_back(pname);
        //meta[key] = data;
    }
}

/*
 *      insertDocuments ---
 *
 *      1.)
 *      Create a project if 'pname' does not exist in the metadata.  This project consists of a name mapped 
 *      to a UUID and a UUID mapped to an array of document ID's.
 *      
 *      2.)
 *      If the JSON object passed in is an array, iterate over the array and insert each document into the DB.
 *      If the JSON object is a single object, insert the document into the DB.
 */

void insertDocuments(rapidjson::Document &docs, std::string &pname, META &meta, FILESYSTEM &fs) {
    updateProjectList(pname, meta);

    rapidjson::Document::AllocatorType &allocator = docs.GetAllocator();

    // If it's an array of documents.  Iterate over them and insert each document.
    if (docs.GetType() == rapidjson::kArrayType) {
        for( auto it = docs.MemberBegin() ; it != docs.MemberEnd() ; it++ ) {
            rapidjson::Value& val = it->value;
            std::string docUUID = newUUID();
            val.AddMember( "_doc" , rapidjson::Value( docUUID.c_str() , allocator) , allocator );
            std::string data = toString( &(it->value));
            insertDocument( docUUID , data , pname, meta, fs);
        }
    } else if (docs.GetType() == rapidjson::kObjectType) {
        std::string docUUID = newUUID();
        docs.AddMember( "_doc" , rapidjson::Value( docUUID.c_str() , allocator) , allocator );
        std::string data = toString(&docs);
        insertDocument( docUUID , data , pname, meta, fs);
    }
}

// Assume doc is an array or an object.
rapidjson::Document processFields(rapidjson::Document &doc, rapidjson::Document &aggregates) {
    rapidjson::Document newDoc;
    newDoc.SetArray();

    // Build an array of unique keys
    for (rapidjson::Value::ConstValueIterator src = doc.Begin(); src != doc.End(); ++src) {
        if (src->IsString()) {
            const char *srcVal = src->GetString();
            //std::string srcVal = src->GetString();
            bool match = false;
            for (rapidjson::Value::ConstValueIterator dest = newDoc.Begin(); dest != newDoc.End(); ++dest) {
                const char* destVal = dest->GetString();
                //std::string destVal = dest->GetString();
                //if (srcVal.compare(destVal) == 0) {
                if (strcmp(srcVal,destVal) == 0) {
                    match = true;
                    break;
                }
            }
            if (!match) {
                rapidjson::Value newVal(srcVal, newDoc.GetAllocator());
                newDoc.PushBack(newVal, newDoc.GetAllocator());
            }
        }
    }

    // Iterate over aggregate objects and check if the field is missing from the selected fields
    for (rapidjson::Value::ConstValueIterator agg = aggregates.Begin(); agg != aggregates.End(); ++agg) {
        bool found = false;
        const rapidjson::Value &obj = *agg;
        //std::string aggVal = obj["field"].GetString();
        const char* aggVal = obj["field"].GetString();
        for (rapidjson::Value::ConstValueIterator src = newDoc.Begin(); src != newDoc.End(); ++src) {
            if (src->IsString()) {
                //std::string srcVal = src->GetString();
                const char* srcVal = src->GetString();
                if ( strcmp(aggVal,srcVal) == 0) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            rapidjson::Value tmpField;      
            tmpField.SetObject();
            rapidjson::Value fieldName(aggVal, newDoc.GetAllocator());
            tmpField.AddMember("_temporary", fieldName, newDoc.GetAllocator());
            newDoc.PushBack(tmpField, newDoc.GetAllocator());
        }
    }

    return newDoc;
}

inline rapidjson::Document extractAggregates(rapidjson::Document &fields ) {

    rapidjson::Document newArray;
    newArray.SetArray();

    for (rapidjson::Value::ConstValueIterator it = fields.Begin(); it != fields.End(); ++it) {
        if (it->GetType() == rapidjson::kObjectType) {
            const rapidjson::Value& v_tmp = *it;
            rapidjson::Value v(v_tmp, newArray.GetAllocator());
            newArray.PushBack(v, newArray.GetAllocator());
        }
    }
    return newArray;
}


// Copy all fields from src to dest
int selectAllFields(rapidjson::Document *src, rapidjson::Value *dest, rapidjson::Document::AllocatorType &allocator) {
    int count = 0;
    rapidjson::Document &doc = *src;
    for (rapidjson::Value::ConstMemberIterator field = doc.MemberBegin(); field != doc.MemberEnd(); ++field) {
        std::string fieldTxt = field->name.GetString();
        rapidjson::Value k(fieldTxt.c_str(), allocator);
        rapidjson::Value &v_tmp = doc[fieldTxt.c_str()];
        rapidjson::Value v(v_tmp, allocator);
        dest->AddMember(k, v, allocator);
        count++;
    }
    return count;
}

// Copy a subset of fields from src to dest
int projectFields(rapidjson::Document *src, rapidjson::Value *dest, rapidjson::Document *fields, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Document &doc = *src;
    int count = 0;
    for (rapidjson::Value::ConstValueIterator field = fields->Begin(); field != fields->End(); ++field) {
        // If the field is an object then it must be an aggregated field
        if (field->GetType() == rapidjson::kObjectType) {
            const rapidjson::Value &obj = *field;
            // Temporary fields are not explicitly selected by the user, but must be included for the aggregation
            if (obj.HasMember("_temporary")) {
                std::string tmpField = obj["_temporary"].GetString();
                if (doc.HasMember(tmpField.c_str())) {
                    rapidjson::Value tempObj;
                    tempObj.SetObject();
                    rapidjson::Value k(tmpField.c_str(), allocator);
                    rapidjson::Value &v_tmp = doc[tmpField.c_str()];
                    rapidjson::Value v(v_tmp, allocator);   
                    tempObj.AddMember("_temporary", v, allocator);
                    dest->AddMember(k, tempObj, allocator);
                    count++;
                }
            }
        } else if (field->GetType() == rapidjson::kStringType) {
            std::string fieldTxt = field->GetString();
            if (doc.HasMember(fieldTxt.c_str())) {
                rapidjson::Value k(fieldTxt.c_str(), allocator);
                rapidjson::Value &v_tmp = doc[fieldTxt.c_str()];
                rapidjson::Value v(v_tmp, allocator);
                dest->AddMember(k, v, allocator);
                count++;
            }
        }
    }
    return count;
}

bool validateSpecialValueCompare(std::string &val) {
    int numSpecials = sizeof(SpecialValueComparisons) / sizeof(SpecialValueComparisons[0]);
    for (int i=0; i<numSpecials; ++i) {
        if (SpecialValueComparisons[i].compare(val) == 0) {
            return true;
        }
    }
    return false;
}

bool validateSpecialKeyCompare(std::string &val) {
    int numSpecials = sizeof(SpecialKeyComparisons) / sizeof(SpecialKeyComparisons[0]);
    for (int i=0; i<numSpecials; ++i) {
        if (SpecialKeyComparisons[i].compare(val) == 0) {
            return true;
        }
    }
    return false;
}

// First is the value from the condition.  It may contain special fields... #gt, #lt
bool sameValues(rapidjson::Value &first, rapidjson::Value &second, rapidjson::Document::AllocatorType &allocator) {
    bool foundSpecial = false;
    std::string specialCompare;
    rapidjson::Value specialValue;

    // Could be a special condition.
    if (first.GetType() == rapidjson::kObjectType && second.GetType() != rapidjson::kObjectType) {
        for (rapidjson::Value::MemberIterator it = first.MemberBegin(); it != first.MemberEnd(); ++it) {
            specialCompare = it->name.GetString();
            if (specialCompare[0] == '#') {
                foundSpecial = true;
                specialValue = rapidjson::Value(first[specialCompare.c_str()], allocator);
                break;
            }
        }
        if (!foundSpecial) {
            return false;
        }
    } else {
        if (first.GetType() != second.GetType()) {
            return false;
        }
    }

    rapidjson::Value condition;
    rapidjson::Type type;
    if (foundSpecial) {

        type = specialValue.GetType();
        if (!validateSpecialValueCompare(specialCompare) || type != second.GetType()) {
            return false;
        }
        condition = rapidjson::Value(specialValue, allocator);
    } else {
        type = first.GetType();
        condition = rapidjson::Value(first, allocator);
    }

    switch (type) {
        case rapidjson::kNullType:
            {
                return true;
                break;
            }
        case rapidjson::kStringType:
            {
                std::string firstStr = condition.GetString();
                std::string secondStr = second.GetString();
                if (foundSpecial) {
                    if (!specialCompare.compare("#gt")) {
                        return secondStr.compare(firstStr) > 0;
                    } else if (!specialCompare.compare("#lt")) {
                        return secondStr.compare(firstStr) < 0;
                    } else if (!specialCompare.compare("#eq")) {
                        return firstStr.compare(secondStr) == 0;
                    } else if (!specialCompare.compare("#contains")) {
                        return secondStr.find(firstStr) != std::string::npos;
                    } else if (!specialCompare.compare("#starts")) {
                        if (secondStr.length() < firstStr.length()) {
                            return false;
                        } else {
                            return secondStr.compare(0, firstStr.length(), firstStr) == 0;
                        }
                    } else if (!specialCompare.compare("#ends")) {
                        if (secondStr.length() < firstStr.length()) {
                            return false;
                        } else {
                            return secondStr.compare(secondStr.length() - firstStr.length(), firstStr.length(), firstStr) == 0;
                        }

                    }
                } else {
                    return firstStr.compare(secondStr) == 0;
                }
                break;
            }
        case rapidjson::kNumberType:
            {
                double firstNum;
                double secondNum;
                if (condition.IsInt()) {
                    firstNum = (double)condition.GetInt();
                } else {
                    firstNum = condition.GetDouble();
                }
                if (second.IsInt()) {
                    secondNum = (double)second.GetInt();
                } else {
                    secondNum = second.GetDouble();
                }

                if (foundSpecial) {
                    if (!specialCompare.compare("#gt")) {
                        return secondNum > firstNum;
                    } else if (!specialCompare.compare("#lt")) {
                        return secondNum < firstNum;
                    } else if (!specialCompare.compare("#eq")) {
                        return secondNum == firstNum;
                    }
                } else {
                    return firstNum == secondNum;
                }
                break;
            }
        case rapidjson::kFalseType:
            {
                //TODO: Maybe #eq should be supported here
                if (foundSpecial) return false;
                return true;
                break;
            }
        case rapidjson::kTrueType:
            {
                //TODO: Maybe #eq should be supported here
                if (foundSpecial) return false;
                return true;
                break;
            }
        case rapidjson::kObjectType: // Special case, compare fields recursively.
            {
                for (rapidjson::Value::MemberIterator it = condition.MemberBegin(); it != condition.MemberEnd(); ++it) {
                    if (!second.HasMember(it->name.GetString())) {
                        return false;
                    }
                    rapidjson::Value &firstEmbed = condition[it->name.GetString()];
                    rapidjson::Value &secondEmbed = second[it->name.GetString()];
                    if (!sameValues(firstEmbed, secondEmbed, allocator)) {
                        return false;
                    }
                }
                return true;
            }
        case rapidjson::kArrayType:
            {
                // TODO: should the special comparisons work on arrays somehow?
                if (foundSpecial) return false;
                if (condition.Size() != second.Size()) {
                    return false;
                }
                for (rapidjson::SizeType i=0; i < condition.Size(); ++i) {
                    if (!sameValues(condition[i], second[i], allocator)) {
                        return false;
                    }
                }
                return true;
            }
    }
    return false;
}



bool documentMatchesConditions(rapidjson::Document &doc, rapidjson::Document &conditions) {
    auto condIt = conditions.MemberBegin();
    while (condIt != conditions.MemberEnd()) {
        std::string condKey = condIt->name.GetString();
        // Special case for special key comparisons
        if (validateSpecialKeyCompare(condKey)) {
            rapidjson::Value &v = conditions[condKey.c_str()];
            if (v.GetType() != rapidjson::kObjectType) {
                return false;
            }
            std::string key = v.MemberBegin()->name.GetString();
            rapidjson::Value &ve = v[key.c_str()];

            // Special case fot exists... 
            if (condKey.compare("#exists") == 0) {
                if (!doc.HasMember(key.c_str()) && ve.GetType() == rapidjson::kTrueType) {
                    return false;
                } else if (doc.HasMember(key.c_str()) && ve.GetType() == rapidjson::kFalseType) {
                    return false;
                } else {
                    continue;
                }
            }

            // Everything else relies on the existence of the key
            if (doc.HasMember(key.c_str())) {
                rapidjson::Value &specVal = doc[key.c_str()];
                if (condKey.compare("#isnull") == 0) {
                    if (specVal.GetType() == rapidjson::kNullType && ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if (specVal.GetType() != rapidjson::kNullType && ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                } else if (condKey.compare("#isstr") == 0) {
                    if (specVal.GetType() == rapidjson::kStringType && ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if (specVal.GetType() != rapidjson::kStringType && ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                } else if (condKey.compare("#isnum") == 0) {
                    if (specVal.GetType() == rapidjson::kNumberType && ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if (specVal.GetType() != rapidjson::kNumberType && ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                } else if (condKey.compare("#isbool") == 0) {
                    if ((specVal.GetType() == rapidjson::kFalseType || specVal.GetType() == rapidjson::kTrueType) && 
                            ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if ((specVal.GetType() != rapidjson::kFalseType && specVal.GetType() != rapidjson::kTrueType) && 
                            ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                } else if (condKey.compare("#isobj") == 0) {
                    if (specVal.GetType() == rapidjson::kObjectType && ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if (specVal.GetType() != rapidjson::kObjectType && ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                } else if (condKey.compare("#isarray") == 0) {
                    if (specVal.GetType() == rapidjson::kArrayType && ve.GetType() == rapidjson::kFalseType) {
                        return false;
                    } else if (specVal.GetType() != rapidjson::kArrayType && ve.GetType() == rapidjson::kTrueType) {
                        return false;
                    }
                }
            } else {
                return false;
            }
        } else {
            if (!doc.HasMember(condKey.c_str())) {
                return false;
            }
            rapidjson::Value &condVal = conditions[condKey.c_str()];
            rapidjson::Value &docVal = doc[condKey.c_str()];
            if (!sameValues(condVal, docVal, conditions.GetAllocator())) {
                return false;
            }
        }
        conditions.RemoveMember(condIt);
    }
    return true;
}

void deleteFields(rapidjson::Document *doc, rapidjson::Document *fields) {
    for (rapidjson::Value::ConstValueIterator it = fields->Begin(); it != fields->End(); it++) {
        const rapidjson::Value &field = *it;
        if (doc->HasMember(field.GetString())) {
            doc->RemoveMember(field.GetString());
        }
    }
}

// Update the fields of the array of documents
void update(DOCDS& docs, rapidjson::Document &updates, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
    if (limit == 0) return;

    int num = 0;
    rapidjson::Document doc;

    // Iterate over every document
    for (auto docID = docs.begin(); docID != docs.end(); ++docID) {
        // Open the document
        const std::string &name = *docID;
        File file1 = fs.open_file( name );
        char *c = fs.read(&file1);
        doc.Parse(c);
        free(c);

        // Parse the document

        if (where) {
            rapidjson::Document &whereDoc = *where;
            rapidjson::Document spare;
            spare.CopyFrom(whereDoc, spare.GetAllocator());
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document.
            if (!documentMatchesConditions(doc, spare)) {
                continue;
            }
        }

        // Insert or update the fields
        for (rapidjson::Value::ConstMemberIterator update = updates.MemberBegin(); update != updates.MemberEnd(); ++update) {
            auto key = update->name.GetString();
            if (doc.HasMember(key)) {
                doc.RemoveMember(key);
            }
            rapidjson::Value k(key, doc.GetAllocator());
            rapidjson::Value &v_tmp = updates[key];
            rapidjson::Value v(v_tmp, doc.GetAllocator());
            doc.AddMember(k,v, doc.GetAllocator());
        }
        //File file2 = fs.open_file(*docID);
        std::string data = toString(&doc);
        fs.write(&file1, data.c_str(), data.size());

        // In case a limit is being used, pre-empt may be necessary
        if (limit > 0 && ++num == limit) {
            break;
        }
    }
}


// Delete the fields from the array of documents
void ddelete(DOCDS& docs, rapidjson::Document &origFields, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
    if (limit == 0) return;

    int num = 0;

    bool selectAll = false;

    rapidjson::Document aggregates = extractAggregates(origFields);
    rapidjson::Document fields = processFields(origFields, aggregates);

    // Check for *
    for (rapidjson::Value::ConstValueIterator it = fields.Begin(); it != fields.End(); ++it) {
        if (it->GetType() != rapidjson::kStringType) continue;
        const char* val = it->GetString();
        if( val[0] == '*'  && val[1] == 0 ) {
            selectAll = true;
            break;
        }
        /*
        if (val.compare("*") == 0) {
            selectAll = true;
        }
        */
    }
    rapidjson::Document doc;

    // Iterate over every document
    auto docID = docs.begin();
    while (docID != docs.end()) {
        // Open the document
        std::string& dID = *docID;
        File file1 = fs.open_file(dID);
        char *c = fs.read(&file1);

        // Parse the document
        doc.Parse(c);
        free(c);

        if (where) {
            rapidjson::Document &whereDoc = *where;
            rapidjson::Document spare;
            spare.CopyFrom(whereDoc, spare.GetAllocator());
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document.
            if (!documentMatchesConditions(doc, spare)) {
                ++docID;
                continue;       
            }
        }

        // Iterate over the desired fields
        if (selectAll) {
            // Delete document 
            bool success = fs.deleteFile(&file1);
            if (success) {
                docs.erase(docID++);
            }
 //           goto next;
        } else {
            deleteFields(&doc, &fields);
            std::string newData = toString(&doc);
            File file2 = fs.open_file(dID);
            fs.write(&file2, newData.c_str(), newData.size());
            ++docID;
        }


//next:
        // If a limit is being used then we may need to pre-empt.
        if (limit > 0 && ++num == limit) {
            break;
        }
    }
}


// Select
rapidjson::Document select(DOCDS &docs, rapidjson::Document &origFields, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
    rapidjson::Document result;
    result.SetObject();

    if (limit == 0) return result;

    rapidjson::Value array;
    array.SetArray();

    array.Reserve(docs.size(), result.GetAllocator());

    bool selectAll = false;

    rapidjson::Document aggregates = extractAggregates(origFields);
    rapidjson::Document fields = processFields(origFields, aggregates);

    // Check for *
    for (rapidjson::Value::ConstValueIterator it = fields.Begin(); it != fields.End(); ++it) {
        if (it->GetType() != rapidjson::kStringType) continue;
        std::string val = it->GetString();
        if (val.compare("*") == 0) {
            selectAll = true;
        }
    }

    int num = 0;
    rapidjson::Document doc;

    // Create Aggregator object
    Aggregator *aggregator = new Aggregator();

    // Iterate over every document
    for (auto docID = docs.begin(); docID != docs.end(); ++docID) {
        // Open the document
        std::string& dID = *docID;
        File file = fs.open_file(dID);
        char *c = fs.read(&file);

        // Parse the document
        doc.Parse(c);
        free(c);

        rapidjson::Value docVal;
        docVal.SetObject();

        if (where) {
            rapidjson::Document &whereDoc = *where;
            rapidjson::Document spare;
            spare.CopyFrom(whereDoc, spare.GetAllocator());
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document
            if (!documentMatchesConditions(doc, spare)) {
                continue;       
            }
        }

        int count = 0;
        // Iterate over the desired fields
        if (selectAll) {
            // Add every field of the document to the result
            count += selectAllFields(&doc, &docVal, result.GetAllocator());
        } else {
            count += projectFields(&doc, &docVal, &fields, result.GetAllocator());
        }

        // Iterate over the aggregates.  Process this document
        for (rapidjson::Value::ConstValueIterator agg = aggregates.Begin(); agg != aggregates.End(); ++agg) {
            // Check if the document contains a field being aggregated.
            // If true:
            //	Pass field to aggregator
            //	If field is marked as temporary, delete it from the document
            // Else:
            //	Continue
            const rapidjson::Value &aggVal = *agg;
            if (!docVal.HasMember(aggVal["field"].GetString())) {
                continue;
            }
            aggregator->handle(&docVal, &aggVal, result.GetAllocator());
            count = docVal.MemberBegin()==docVal.MemberEnd()?0:count;
        }

        if (count > 0) {
            array.PushBack(docVal, result.GetAllocator());
        }

        // If a limit is being used then we may need to preempt.
        if (++num == limit) {
            break;
        }
    }

    for (rapidjson::Value::ConstValueIterator agg = aggregates.Begin(); agg != aggregates.End(); ++agg) {
        const rapidjson::Value &a = *agg;
        std::string func = a["function"].GetString();
        std::string field = a["field"].GetString();
        AggregateResult *res = aggregator->getResult(field, func);
        if (res != NULL) {
            rapidjson::Value aggRes;
            aggRes.SetObject();
            std::string k_txt = func + '(' + field + ')';
            rapidjson::Value k(k_txt.c_str(), result.GetAllocator());
            rapidjson::Value v(res->result);
            aggRes.AddMember(k, v, result.GetAllocator());
            array.PushBack(aggRes, result.GetAllocator());
            delete res;
        }
    }
    delete aggregator;

    result.AddMember("_result", array, result.GetAllocator());

    // Create a timestamp
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    char buffer[80];
    strftime(buffer,80,"%F %X",timeinfo);

    // Add timestamp to result
    rapidjson::Value tstamp(buffer, result.GetAllocator());
    result.AddMember("_timestamp", tstamp, result.GetAllocator());

    return result;
}

/*
 *      execute ---
 *      
 *      Given a parsed query, execute this query and perform the appropriate CRUD operation.
 *      Display the results.
 *
 */

void execute(Parsing::Query *q, META *m, FILESYSTEM *f, bool print = true) {
    clock_t start, end;
    start = std::clock();

    META &meta = *m;
    FILESYSTEM &fs = *f;

    switch (q->command) {
        case Parsing::CREATE:
            {
                // TODO: Create an index...
                //q.print();
                break;
            }
        case Parsing::INSERT:
            {
                // Insert documents in docs into project.
		if (q->with) {
    			rapidjson::Document with;
			with.CopyFrom(*(q->with), with.GetAllocator());
                	insertDocuments(with, *q->project, meta, fs);
		}
                break;
            }
        case Parsing::SELECT:
            {
                std::string project = *q->project;
                if (meta.count(project) > 0) {
                    rapidjson::Document data = select(meta[project], *q->fields, q->where, q->limit, fs);
                    if (data.HasMember("_result")) {
                        rapidjson::Value &array = data["_result"];
                        if (array.Size() == 0) {
			    PRINT("Result Empty!\r\n");
                        } else {
			    std::string pretty_data = toPrettyString(&data);
			    PRINT(pretty_data, "\r\n");
			    PRINT(array.Size(), " records returned.\r\n");
                        }
                    }
                } else {
		    PRINT("Project '", project, "' does not exist!\r\n");
                }
                break;
            }
        case Parsing::DELETE:
            {
                std::string project = *q->project;
                if (meta.count(project)) {
                    DOCDS& docs = meta[project];
                    ddelete(docs, *q->fields, q->where, q->limit, fs);
                    //meta[project] = docs;
                } else {
		    PRINT("Project '", project, "' does not exist!\r\n");
                }
                break;
            }
        case Parsing::SHOW:
            {
                std::string key("__PROJECTS__");
                if (meta.count(key)) {
                    DOCDS& list = meta[key];
		    PRINT("[\r\n");
                    for (auto it = list.begin() ; it != list.end() ; ++it) {
		        PRINT(*it, "\n");
                    }
		    PRINT("]\r\n");
                } else {
		    PRINT("No projects found!\r\n");
                }
                break;
            }
        case Parsing::UPDATE:
            {
                std::string project = *q->project;
                if (meta.count(project)) {
                    DOCDS& docs = meta[project];
                    rapidjson::Document &updates = *q->with;
                    update( docs, updates, q->where, q->limit, fs);
                    meta[project] = docs;
                } else {
		    PRINT("Project '", project, "' does not exist!\r\n");
                }
                break;
            }
        default:
	    PRINT("Command not recognized!\r\n");
    }
    if (print) {
        end = std::clock();
        double time = (double)(end - start) / CLOCKS_PER_SEC;
        if( time > 1.0 ) {
	    PRINT("Done!  Took ", time, " seconds.\n");
        }else {
	    PRINT("Done!  Took ", 1000 * time, " milliseonds.\n");
        }
    }
    delete q;
}

std::vector<std::string> split(const char *str, char c = ' ') {
    std::vector<std::string> result;
    do {
        const char *begin = str;
        while(*str != c && *str) {
            str++;
        }
        result.push_back(std::string(begin, str));
    } while ('\0' != *str++);

    result.pop_back();

    return result;
}

void completion(const char *buf, linenoiseCompletions *lc) {
    std::vector<std::string> tokens = split(buf);
    int size = 0;
    const std::string *options;
    std::string prefix;

    if (tokens.size() == 0) {
        size = sizeof(Parsing::Commands) / sizeof(Parsing::Commands[0]);
        options = &Parsing::Commands[0];
        prefix = "";
    } else if (tokens.size() == 1) {
        toLower(tokens[0]);
        if (tokens[0] == "create") {
            size = sizeof(Parsing::CreateArgs) / sizeof(Parsing::CreateArgs[0]);
            options = &Parsing::CreateArgs[0];
            prefix = "CREATE ";
        } else if (tokens[0] == "insert") {
            size = sizeof(Parsing::InsertArgs) / sizeof(Parsing::InsertArgs[0]);
            options = &Parsing::InsertArgs[0];
            prefix = "INSERT ";
        } else if (tokens[0] == "select") {
            size = sizeof(Parsing::SelectArgs) / sizeof(Parsing::SelectArgs[0]);
            options = &Parsing::SelectArgs[0];
            prefix = "SELECT ";
        } else if (tokens[0] == "delete") {
            size = sizeof(Parsing::DeleteArgs) / sizeof(Parsing::DeleteArgs[0]);
            options = &Parsing::DeleteArgs[0];
            prefix = "DELETE ";
        } else if (tokens[0] == "update") {
            size = sizeof(Parsing::UpdateArgs) / sizeof(Parsing::UpdateArgs[0]);
            options = &Parsing::UpdateArgs[0];
            prefix = "UPDATE ";
        }
    }

    std::string stuff = prefix;
    for (int i=0 ; i<size ; ++i) {
        linenoiseAddCompletion(lc, (stuff + options[i]).c_str());
    }
}

int main(int argc, char **argv) {
    std::string data_fname("data.db");

    Storage::Filesystem *fs = new Storage::Filesystem(data_fname);
    META *meta;
    File meta_file = fs->open_file("__DB_METADATA__");
    Storage::HerpmapReader<DOCDS,Num_Buckets> meta_reader(meta_file, fs);
    if (meta_file.size > 0) {
        meta = new META(meta_reader.read());
    } else {
        meta = new META();

    }

    int count = 0;

    std::string line;
    char *buf = NULL;
    std::string queryLogFile("queries.log");
    uint64_t origNumFiles = fs->getNumFiles();

    if (argc > 1) {
        std::ifstream dataFile;
        dataFile.open(argv[1]);
        int total = std::count(std::istreambuf_iterator<char>(dataFile), 
                std::istreambuf_iterator<char>(), '\n');
        dataFile.seekg(0);
        double percent = 0;
        while (dataFile.good()) {
            line.clear();
            std::getline(dataFile, line);
            if (line.compare("q") == 0) {
                std::cout << std::endl;
                goto end;
            }
            Parsing::Parser p(line);
            Parsing::Query *query = p.parse();
            if (query) {
#if THREADING
		pool.enqueue( [=] {
#endif
			execute(query, meta, fs, false);
#if THREADING
		});
#endif
                count++;
                percent = (double)count / total;
            }
            printf("%.1f%% done.\r", ceil(percent*100));
        }

	std::cout << std::endl;

	
	while(1) {
		uint64_t n = pool.numTasks();
		std::cout << n << "\r";
		if (n == 1) {
			break;
		}
        }

        std::cout << "\n";
    }

    std::cout << "Welcome to VrbskyDB v" << MAJOR_VERSION << "." << MINOR_VERSION  << std::endl;

    if (file_exists(queryLogFile.c_str())) {
        linenoiseHistoryLoad(queryLogFile.c_str());
    } else {
        std::fstream out(queryLogFile.c_str(), std::fstream::out);
        out.flush();
        out.close();
    }

    linenoiseSetCompletionCallback(completion);

    std::cout << "Enter a query (q to quit):" << std::endl;
    while (1) {
        buf = linenoise("> ");
        if (buf == NULL || strcmp(buf, "q") == 0) {
            break;
        }

        std::string q(buf);
        Parsing::Parser p(q);
        Parsing::Query *query = p.parse();
        if (query) {
            // If the query parses, add it to a log.
            linenoiseHistoryAdd(buf);
            linenoiseHistorySave(queryLogFile.c_str());
#if THREADING
	    pool.enqueue( [=] {
#endif
		execute(query, meta, fs);
#if THREADING
	    });
#endif
        }
        free(buf);
    }

end:

    Storage::HerpmapWriter<DOCDS,Num_Buckets> meta_writer(meta_file, fs);
    meta_writer.write(*meta);
    std::cout << "Goodbye!" << std::endl;
    free(buf);

    // The number of files decresed then compact the fs.
    if (fs->getNumFiles() < origNumFiles) {
        clock_t start, end;
        start = std::clock();
        fs->compact();
        end = std::clock();
        double time = (double)(end-start) / CLOCKS_PER_SEC;
        if( time > 1.0 ) {
            std::cout << "Compaction  Took " << time << " seconds." << std::endl;
        }else {
            std::cout << "Compaction  Took " << 1000 * time << " milliseconds." << std::endl;
        }
    }
    fs->shutdown();

    delete fs;
    delete meta;

    return 0;
}
