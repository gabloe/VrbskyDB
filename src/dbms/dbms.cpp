#include <fstream> 
#include <sstream> 
#include <limits>
#include <cstddef>
#include <time.h>
#include <map>
#include <math.h>
#include <ctime>
#include <vector>

#include "dbms.h"
#include "../hashing/Hash.h"
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"
#include "../mmap_filesystem/Filesystem.h"

#include "../mmap_filesystem/HerpmapWriter.h"
#include "../mmap_filesystem/HerpmapReader.h"
//#include "../mmap_filesystem/HashmapWriter.h"
//#include "../mmap_filesystem/HashmapReader.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <pretty.h>
#include <UUID.h>

// Add val to current value of result.
void sumAggregate(rapidjson::Value *val, rapidjson::Value &result) {
    if (result.IsNull()) {
        result.SetDouble(0.0);
    }

    double x;
    if (val->IsInt()) {
        x = (double)val->GetInt();
    } else {
        x = val->GetDouble();
    }
    result = rapidjson::Value(result.GetDouble() + x);
}

// Update result only if val is smaller
void minAggregate(rapidjson::Value *val, rapidjson::Value &result) {
    if (result.IsNull()) {
        result.SetDouble(std::numeric_limits<double>::max());
    }

    double x;
    if (val->IsInt()) {
        x = (double)val->GetInt();
    } else {
        x = val->GetDouble();
    }

    if (x < result.GetDouble()) {
        result = rapidjson::Value(x);
    }
}

// Update result only if val is larger
void maxAggregate(rapidjson::Value *val, rapidjson::Value &result) {
    if (result.IsNull()) {
        result.SetDouble(std::numeric_limits<double>::min());
    }

    double x;
    if (val->IsInt()) {
        x = (double)val->GetInt();
    } else {
        x = val->GetDouble();
    }

    if (x > result.GetDouble()) {
        result = rapidjson::Value(x);
    }
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
        std::vector<std::string> data;
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

void insertDocument(rapidjson::Document &doc, std::string &project, META &meta, FILESYSTEM &fs) {
    std::string docUUID = newUUID();

    rapidjson::Value dID(docUUID.c_str(), doc.GetAllocator());
    doc.AddMember("_doc", dID, doc.GetAllocator());
    std::string data = toString(&doc);

    // Insert this row into the DB
    File file = fs.open_file(docUUID.c_str());
    fs.write(&file, data.c_str(), data.size());

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
        std::vector<std::string> data;
        data.push_back(pname);
        meta[key] = data;
        return;
    }
    std::vector<std::string>& data = meta[key];

    // Check if the project already exists
    bool found = false;
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::string v = *it;
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

void insertDocuments(rapidjson::Document &docs, std::string pname, META &meta, FILESYSTEM &fs) {
    updateProjectList(pname, meta);

    // If it's an array of documents.  Iterate over them and insert each document.
    if (docs.GetType() == rapidjson::kArrayType) {
        for (rapidjson::SizeType i = 0; i < docs.Size(); ++i) {
            rapidjson::Document d;
            std::string data = toString(&docs[i]);
            d.Parse(data.c_str());
            insertDocument(d, pname, meta, fs);
        }
    } else if (docs.GetType() == rapidjson::kObjectType) {
        insertDocument(docs, pname, meta, fs);
    }
}

// Assume doc is an array or an object.
rapidjson::Document processFields(rapidjson::Document &doc, rapidjson::Document &aggregates) {
    rapidjson::Document newDoc;
    newDoc.SetArray();

    // Build an array of unique keys
    for (rapidjson::Value::ConstValueIterator src = doc.Begin(); src != doc.End(); ++src) {
        if (src->IsString()) {
            std::string srcVal = src->GetString();
            bool match = false;
            for (rapidjson::Value::ConstValueIterator dest = newDoc.Begin(); dest != newDoc.End(); ++dest) {
                std::string destVal = dest->GetString();
                if (srcVal.compare(destVal) == 0) {
                    match = true;
                    break;
                }
            }
            if (!match) {
                rapidjson::Value newVal(srcVal.c_str(), newDoc.GetAllocator());
                newDoc.PushBack(newVal, newDoc.GetAllocator());
            }
        }
    }

    // Iterate over aggregate objects and check if the field is missing from the selected fields
    for (rapidjson::Value::ConstValueIterator agg = aggregates.Begin(); agg != aggregates.End(); ++agg) {
        bool found = false;
        const rapidjson::Value &obj = *agg;
        std::string aggVal = obj["field"].GetString();
        for (rapidjson::Value::ConstValueIterator src = newDoc.Begin(); src != newDoc.End(); ++src) {
            if (src->IsString()) {
                std::string srcVal = src->GetString();
                if (aggVal.compare(srcVal) == 0) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            rapidjson::Value tmpField;      
            tmpField.SetObject();
            rapidjson::Value fieldName(aggVal.c_str(), newDoc.GetAllocator());
            tmpField.AddMember("_temporary", fieldName, newDoc.GetAllocator());
            newDoc.PushBack(tmpField, newDoc.GetAllocator());
        }
    }

    return newDoc;
}

rapidjson::Document extractAggregates(rapidjson::Document &fields) {

    rapidjson::Document newArray;
    newArray.SetArray();

    for (rapidjson::Value::ConstValueIterator it = fields.Begin(); it != fields.End(); ++it) {
        if (it->GetType() == rapidjson::kObjectType) {
            const rapidjson::Value &v_tmp = *it;
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

void aggregateField(rapidjson::Value *result, rapidjson::Value *data, std::string function) {
    if (!data->IsNumber()) return;
    std::map<std::string, std::function<void(rapidjson::Value*,rapidjson::Value&)>>  funcMap =
    {{ "SUM", sumAggregate},
        { "AVG", sumAggregate}, // Special case
        { "MIN", minAggregate},
        { "MAX", maxAggregate}
    };
    funcMap[function.c_str()](data, *result);
    double x = result->GetDouble();

    // Convert the result to an integer if it should be.
    if (fmod(x, 1) == 0.0) {
        *result = rapidjson::Value((int)x);
    }
}

// For each result in src, apply aggregate function.  Return value.
rapidjson::Value processAggregate(rapidjson::Value *src, const rapidjson::Value *func, rapidjson::Document::AllocatorType &allocator) {
    rapidjson::Value result;
    rapidjson::Value &array = *src;
    const rapidjson::Value &aggregate = *func;

    // Get the function and field name to aggregate
    std::string function = aggregate["function"].GetString();
    std::string field = aggregate["field"].GetString();

    int count = 0;
    for (rapidjson::Value::ValueIterator it = array.Begin(); it != array.End(); ++it) {
        rapidjson::Value &obj = *it;

        if (obj.HasMember(field.c_str())) {
            rapidjson::Value embValue;
            rapidjson::Value &embObject = obj[field.c_str()];

            if (embObject.GetType() == rapidjson::kObjectType && embObject.HasMember("_temporary")) {
                rapidjson::Value &tmp = embObject["_temporary"];
                embValue = rapidjson::Value(tmp, allocator);
            } else {
                embValue = rapidjson::Value(embObject, allocator);
            }

            aggregateField(&result, &embValue, function);
            count++;
        }
    }

    // Special case, handle average aggregate.  Is there a better way to do this?
    if (function.compare("AVG") == 0 && count > 0) {
        result = rapidjson::Value(result.GetDouble() / count);
    }

    // Bundle up the result in an object.
    rapidjson::Value obj;
    obj.SetObject();
    std::string objKey(function + "(" + field + ")");
    rapidjson::Value k(objKey.c_str(), allocator);
    rapidjson::Value v(result, allocator);
    obj.AddMember(k, v, allocator);
    return obj;
}

bool validateSpecialValueCompare(std::string val) {
    int numSpecials = sizeof(SpecialValueComparisons) / sizeof(SpecialValueComparisons[0]);
    for (int i=0; i<numSpecials; ++i) {
        if (SpecialValueComparisons[i].compare(val) == 0) {
            return true;
        }
    }
    return false;
}

bool validateSpecialKeyCompare(std::string val) {
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
    for (rapidjson::Value::ConstMemberIterator condIt = conditions.MemberBegin(); condIt != conditions.MemberEnd(); ++condIt) {
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
void update(std::vector<std::string>* docs, rapidjson::Document &updates, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
    if (limit == 0) return;

    int num = 0;

    // Iterate over every document
    for (auto docID = docs->begin(); docID != docs->end(); ++docID) {
        // Open the document
        std::string dID = *docID;
        File file1 = fs.open_file(dID);
        char *c = fs.read(&file1);
        std::string docTxt = std::string(c,file1.size);
        free(c);

        // Parse the document
        rapidjson::Document doc;
        doc.Parse(docTxt.c_str());

        if (where) {
            rapidjson::Document &whereDoc = *where;
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document.
            if (!documentMatchesConditions(doc, whereDoc)) {
                continue;
            }
        }

        // Insert or update the fields
        for (rapidjson::Value::ConstMemberIterator update = updates.MemberBegin(); update != updates.MemberEnd(); ++update) {
            std::string key = update->name.GetString();
            if (doc.HasMember(key.c_str())) {
                doc.RemoveMember(key.c_str());
            }
            rapidjson::Value k(key.c_str(), doc.GetAllocator());
            rapidjson::Value &v_tmp = updates[key.c_str()];
            rapidjson::Value v(v_tmp, doc.GetAllocator());
            doc.AddMember(k,v, doc.GetAllocator());
        }
        File file2 = fs.open_file(dID);
        std::string data = toString(&doc);
        fs.write(&file2, data.c_str(), data.size());

        // In case a limit is being used, pre-empt may be necessary
        if (limit > 0 && ++num == limit) {
            break;
        }
    }
}


// Delete the fields from the array of documents
void ddelete(std::vector<std::string> *docs, rapidjson::Document &origFields, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
    if (limit == 0) return;

    int num = 0;

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

    // Iterate over every document
    auto docID = docs->begin();
    while (docID != docs->end()) {
        // Open the document
        std::string dID = *docID;
        File file1 = fs.open_file(dID);
        char *c = fs.read(&file1);
        std::string docTxt = std::string(c,file1.size);
        free(c);

        // Parse the document
        rapidjson::Document doc;
        doc.Parse(docTxt.c_str());

        if (where) {
            rapidjson::Document &whereDoc = *where;
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document.
            if (!documentMatchesConditions(doc, whereDoc)) {
                ++docID;
                continue;       
            }
        }

        // Iterate over the desired fields
        if (selectAll) {
            // Delete document 
            bool success = fs.deleteFile(&file1);
            if (success) {
                docs->erase(docID);
            }
            goto next;
        } else {
            deleteFields(&doc, &fields);
            std::string newData = toString(&doc);
            File file2 = fs.open_file(dID);
            fs.write(&file2, newData.c_str(), newData.size());
        }

        ++docID;

next:
        // If a limit is being used then we may need to pre-empt.
        if (limit > 0 && ++num == limit) {
            break;
        }
    }
}

rapidjson::Document select(std::vector<std::string> docs, rapidjson::Document &origFields, rapidjson::Document *where, int limit, FILESYSTEM &fs) {
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

    // Iterate over every document
    for (auto docID = docs.begin(); docID != docs.end(); ++docID) {
        // Open the document
        std::string dID = *docID;
        File file = fs.open_file(dID);
        char *c = fs.read(&file);
        std::string docTxt = std::string(c,file.size);
        free(c);

        // Parse the document
        rapidjson::Document doc;
        doc.Parse(docTxt.c_str());

        rapidjson::Value docVal;
        docVal.SetObject();

        if (where) {
            rapidjson::Document &whereDoc = *where;
            // Check if the document contains the values specified in where clause.
            // If not, move on to the next document.
            if (!documentMatchesConditions(doc, whereDoc)) {
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
        if (count > 0) {
            array.PushBack(docVal, result.GetAllocator());
        }

        // If a limit is being used then we may need to preempt.
        if (limit > 0 && ++num == limit) {
            break;
        }
    }

    for (rapidjson::Value::ConstValueIterator agg = aggregates.Begin(); agg != aggregates.End(); ++agg) {
        const rapidjson::Value &aggVal = *agg;
        rapidjson::Value aggResult = processAggregate(&array, &aggVal, result.GetAllocator());
        array.PushBack(aggResult, result.GetAllocator());
    }

    rapidjson::Value::ValueIterator tmp = array.Begin();
    while (tmp != array.End()) {
        rapidjson::Value &tmpObj = *tmp;
        rapidjson::Value::MemberIterator tmpField = tmpObj.MemberBegin();
        int numFields = 0;
        int numDeleted = 0;
        while (tmpField != tmpObj.MemberEnd()) {
            std::string key = tmpField->name.GetString();
            rapidjson::Value &embedded = tmpObj[key.c_str()];
            if (embedded.GetType() == rapidjson::kObjectType && embedded.HasMember("_temporary")) {
                tmpObj.RemoveMember(tmpField);
                numDeleted++;
            } else {
                tmpField++;
            }
            numFields++;
        }
        if (numDeleted == numFields) {
            array.Erase(tmp);
        } else {
            tmp++;
        }
    }

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

void execute(Parsing::Query &q, META &meta, FILESYSTEM &fs, bool print = true) {
    clock_t start, end;
    start = std::clock();
    switch (q.command) {
        case Parsing::CREATE:
            {
                // TODO: Create an index...
                q.print();
                break;
            }
        case Parsing::INSERT:
            {
                rapidjson::Document &docs = *q.with;
                std::string project = *q.project;
                // Insert documents in docs into project.
                insertDocuments(docs, project, meta, fs);
                break;
            }
        case Parsing::SELECT:
            {
                std::string project = *q.project;
                if (meta.count(project) > 0) {
                    rapidjson::Document data = select(meta[project], *q.fields, q.where, q.limit, fs);
                    if (data.HasMember("_result")) {
                        rapidjson::Value &array = data["_result"];
                        if (array.Size() == 0) {
                            std::cout << "Result Empty!" << std::endl;	
                        } else {
                            std::cout << toPrettyString(&data) << std::endl;
                            std::cout << array.Size() << " records returned." << std::endl; 
                        }
                    }
                } else {
                    std::cout << "Project '" << project << "' does not exist!" << std::endl;
                }
                break;
            }
        case Parsing::DELETE:
            {
                std::string project = *q.project;
                if (meta.count(project)) {
                    std::vector<std::string> docs = meta[project];
                    ddelete(&docs, *q.fields, q.where, q.limit, fs);
                    meta[project] = docs;
                } else {
                    std::cout << "Project '" << project << "' does not exist!" << std::endl;
                }
                break;
            }
        case Parsing::SHOW:
            {
                std::string key("__PROJECTS__");
                if (meta.count(key)) {
                    std::vector<std::string> list = meta[key];
                    std::cout << "[" << std::endl;
                    for (auto it = list.begin() ; it != list.end() ; ++it) {
                        std::cout << "\t" << *it << std::endl;
                    }
                    std::cout << "]" << std::endl;
                } else {
                    std::cout << "No projects found!" << std::endl;
                }
                break;
            }
        case Parsing::UPDATE:
            {
                std::string project = *q.project;
                if (meta.count(project)) {
                    std::vector<std::string> docs = meta[project];
                    rapidjson::Document &updates = *q.with;
                    update(&docs, updates, q.where, q.limit, fs);
                    meta[project] = docs;
                } else {
                    std::cout << "Project '" << project << "' does not exist!" << std::endl;
                }
                break;
            }
        default:
            std::cout << "Command not recognized." << std::endl;
    }
    if (print) {
        end = std::clock();
        std::cout << "Done!  Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << " milliseconds." << std::endl;
    }
    //dumpToFile(meta_fname, meta);
}

/*
 *      Begin autocompletion methods for Readline library.
 */

int myNewline(int UNUSED(count), int UNUSED(key)) {
    if (rl_line_buffer[strlen(rl_line_buffer)-1] == ';' ||
            strcmp(rl_line_buffer, "q") == 0) {
        rl_done = 1;
        std::cout << "\n";
    } else { 
        std::cout << "\n> ";
    }
    return 0;
}

void * xmalloc (int size) {
    void *buf;

    buf = malloc (size);
    if (!buf) {
        fprintf (stderr, "Error: Out of memory. Exiting.'n");
        exit (1);
    }

    return buf;
}

char * dupstr (char* s) {
    char *r;

    r = (char*) xmalloc ((strlen (s) + 1));
    strcpy (r, s);
    return (r);
}


char *matcher( const char *text , int state, unsigned int &list_index , unsigned int &len , const std::string stuff[] , unsigned int length ) {
    char *name;
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < length) {
        name = (char *)stuff[list_index++].c_str();
        if (!strncasecmp(name, text, len)) {
            return (dupstr(name));
        }
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}


char *cmdMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::Commands , LENGTH(Parsing::Commands) );
}

char *createMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::CreateArgs , LENGTH(Parsing::CreateArgs) );
}

char *selectFromMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::SelectFromArgs , LENGTH(Parsing::SelectFromArgs) );
}

char *selectMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::SelectArgs , LENGTH(Parsing::SelectArgs) );
}

char *insertMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::InsertArgs , LENGTH(Parsing::InsertArgs) );
}

char *insertIntoMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::InsertIntoArgs , LENGTH(Parsing::InsertIntoArgs) );
}

char *showMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::ShowArgs , LENGTH(Parsing::ShowArgs) );
}

char *deleteMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::DeleteArgs , LENGTH(Parsing::DeleteArgs) );
}

char *deleteFromMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::DeleteFromArgs , LENGTH(Parsing::DeleteFromArgs) );
}

char *updateMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::UpdateArgs , LENGTH(Parsing::UpdateArgs) );
}

char *updateWithMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::UpdateWithArgs , LENGTH(Parsing::UpdateWithArgs) );
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

    return result;
}

static char **myAutoComplete(const char * text, int start, int UNUSED(end)) {
    char **matches = (char **)NULL;
    if (start == 0) {
        matches = rl_completion_matches((char *)text, &cmdMatcher);
    } else {
        // TODO: Make this more robust...

        char *copy = (char *)malloc(strlen(rl_line_buffer) + 1);
        strcpy(copy, rl_line_buffer);

        std::vector<std::string> tokens = split((const char *)copy);

        if (!strcasecmp(tokens[0].c_str(), "create")) {
            if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &createMatcher);
            }
        } else if (!strcasecmp(tokens[0].c_str(), "select")) {
            if (tokens.size() == 2) {       
                matches = rl_completion_matches((char *)tokens.back().c_str(), &selectMatcher);
            } else if (tokens.size() == 4) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &selectFromMatcher);
            }
        } else if (!strcasecmp(tokens[0].c_str(), "insert")) {
            if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &insertMatcher);
            } else if (tokens.size() == 4) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &insertIntoMatcher);
            }
        } else if (!strcasecmp(tokens[0].c_str(), "delete")) {
            if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &deleteMatcher);
            } else if (tokens.size() == 4) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &deleteFromMatcher);
            }
        } else if (!strcasecmp(tokens[0].c_str(), "update")) {
            if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &updateMatcher);
            } else if (tokens.size() == 4) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &updateWithMatcher);
            }
        }

        free(copy);
    }

    return matches;
}

int myCompletion(int UNUSED(count), int UNUSED(key)) {
    return rl_complete_internal('@');
}

char *myDummy(const char *UNUSED(count), int UNUSED(key)) {
    return (char*)NULL;
}

int start_readline(void) {
    // Enable limited autocomplete
    rl_attempted_completion_function = myAutoComplete;
    rl_completion_entry_function = myDummy;
    rl_bind_key('\t', myCompletion);    

    // Handle multi-line input
    rl_bind_key('\r', myNewline);
    rl_bind_key('\n', myNewline);

    return 0;
}


int main(int argc, char **argv) {
    std::string data_fname("data.db");
    char *buf;
    Storage::Filesystem *fs = new Storage::Filesystem(data_fname);
    META *meta;
    File meta_file = fs->open_file("__DB_METADATA__");
    Storage::HerpmapReader<std::vector<std::string>> meta_reader(meta_file, fs);
    if (meta_file.size > 0) {
        //meta = new std::map<std::string, std::string>(meta_reader.read());
        meta = new Storage::HerpHash<std::string, std::vector<std::string>>(meta_reader.read());
    } else {
        //meta = new std::map<std::string, std::string>();
        meta = new Storage::HerpHash<std::string, std::vector<std::string>>();
    }
    std::string line;
    int count = 0;
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
            Parsing::Parser p(line);
            Parsing::Query *query = p.parse();
            if (query) {
                execute( *query, *meta, *fs, false);
                count++;
                percent = (double)count / total;
                delete query;
            }
            printf("%.1f%% done.\r", ceil(percent*100));
        }
        std::cout << "\n";
    }

    std::cout << "Welcome to VrbskyDB v" << MAJOR_VERSION << "." << MINOR_VERSION  << std::endl;

    rl_startup_hook = start_readline;

    std::string queryLogFile("queries.log");

    if (file_exists(queryLogFile.c_str())) {
        read_history(queryLogFile.c_str());
    } else {
        std::fstream out(queryLogFile.c_str(), std::fstream::out);
        out.flush();
        out.close();
    }

    uint64_t origNumFiles = fs->getNumFiles();

    std::cout << "Enter a query (q to quit):" << std::endl;
    while (1) {
        buf = readline("> ");

        if (buf == NULL) {
            break;
        }
        if (buf[0] != 0) {
            add_history(buf);
        }

        std::string q(buf);
        if (!q.compare("q")) {
            break;
        }

        Parsing::Parser p(q);
        Parsing::Query *query = p.parse();
        if (query) {
            // If the query parses, add it to a log.
            append_history(1, queryLogFile.c_str());
            execute( *query, *meta, *fs );
            delete query;
        }
        std::cout << std::endl;
    }
    Storage::HerpmapWriter<std::vector<std::string>> meta_writer(meta_file, fs);
    meta_writer.write(*meta);
    std::cout << "Goodbye!" << std::endl;
    free(buf);

    // The number of files decresed then compact the fs.
    if (fs->getNumFiles() < origNumFiles) {
        clock_t start, end;
        start = std::clock();
        fs->compact();
        end = std::clock();
        std::cout << "Compaction  Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << " milliseconds." << std::endl;
    }
    fs->shutdown();

    delete fs;
    delete meta;

    return 0;
}
