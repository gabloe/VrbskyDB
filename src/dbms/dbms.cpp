#include <fstream> 
#include <sstream> 
#include <limits>
#include <cstddef>

#include "../storage/LinearHash.h"
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <UUID.h>

#define LENGTH(A) sizeof(A)/sizeof(A[0])

// TODO: Why are these not in a header file?
void execute(Parsing::Query &, Storage::LinearHash<std::string> &);
std::string toPrettyString(std::string *);
std::string toPrettyString(rapidjson::Document *);

#define UNUSED(id)

// Convert a JSON object to a std::string
std::string toString(rapidjson::Document *doc) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    std::string str = buffer.GetString();
    return str;
}


std::string toPrettyString(std::string doc) {
    rapidjson::Document d;
    d.Parse(doc.c_str());
    return toPrettyString(&d);
}

std::string toPrettyString(char *doc) {
    return toPrettyString(std::string(doc));
}

std::string toPrettyString(rapidjson::Document *doc) {
    rapidjson::StringBuffer out;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(out);
    doc->Accept(writer);
    return out.GetString();
}

// Given a uuid return a document if it exists
rapidjson::Document readData(std::string dataFile, std::string uuid, Storage::LinearHash<uint64_t> &indices) {
    uint64_t key = hash(uuid, uuid.size());
    uint64_t ind = 0;
    indices.get(key,ind);
    std::ifstream in(dataFile.c_str(), std::ios::in | std::ios::binary);
    in.seekg(ind);

    size_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(size_t));
    std::string ret(len, '\0'); 
    in.read(&ret[0], len);
    in.close();
    rapidjson::Document doc;
    doc.Parse(ret.c_str());
    doc.RemoveMember("__UUID__");
    return doc;
}

//  Given a project name we return a list of all UUID values stored in it
Parsing::List<std::string> *extractUUIDArray(std::string project,
        std::string document, 
        Parsing::List<std::string> *fieldList,
        Storage::LinearHash<std::string> &meta) {

    Parsing::List<std::string> *uuidList = NULL; 
    Parsing::List<std::string> *spot = NULL;

    while (fieldList) {
        // Don't count aggregate functions
        if (fieldList->aggregate != NULL) {
            fieldList = fieldList->next;
            continue;
        }
        std::string key(project + "." + document);
        if (fieldList->value.compare("*") != 0) {
            key += "." + fieldList->value;
        }
        uint64_t keyHash = hash(key, key.size());
        if (meta.contains(keyHash)) {   // If project exists
            std::string fields;
            meta.get(keyHash,fields);

            rapidjson::Document doc;

            doc.Parse(fields.c_str());
            rapidjson::Value &fieldArray = doc["__FIELDS__"];

            rapidjson::Value::ConstValueIterator itr = fieldArray.Begin();
            if (uuidList == NULL) {
                uuidList = new Parsing::List<std::string>((itr++)->GetString());
                spot = uuidList;
            }
            while (itr != fieldArray.End()) {
                Parsing::List<std::string> *node = new Parsing::List<std::string>((itr++)->GetString());
                spot->append(node);
            }
        }
        fieldList = fieldList->next;
    }
    return uuidList;
}

// Create a JSON object containing a project name and list of documents (one document initially)
rapidjson::Document createProject(std::string &name, Parsing::List<std::string> *doc) {
    rapidjson::Document project;
    project.SetObject();
    rapidjson::Document::AllocatorType& allocator = project.GetAllocator();
    rapidjson::Value documentArray(rapidjson::kArrayType);
    rapidjson::Value projectName;
    projectName.SetString(name.c_str(), allocator);
    while (doc) {
        rapidjson::Value document;
        document.SetString(doc->value.c_str(), allocator);
        documentArray.PushBack(document, allocator);
        doc = doc->next;
    }
    project.AddMember("__NAME__", projectName, allocator);
    project.AddMember("__DOCUMENTS__", documentArray, allocator);
    return project;
}

// Given a JSON string, parse the JSON object and append a document name to the array.
rapidjson::Document appendDocument(std::string &value, Parsing::List<std::string> *doc) {
    rapidjson::Document project;
    project.Parse(value.c_str());
    rapidjson::Document::AllocatorType& allocator = project.GetAllocator();
    if (project.HasMember("__DOCUMENTS__")) {
        rapidjson::Value &documentArray = project["__DOCUMENTS__"];
        if (doc) {
            std::string doc_name = doc->value;
            for (rapidjson::Value::ConstValueIterator itr = documentArray.Begin(); itr != documentArray.End(); ++itr) {
                std::string d = itr->GetString();
                if (!d.compare(doc_name)) {
                    std::string error("Document " + doc_name + " already exists in project " + project["__NAME__"].GetString());
                    throw std::runtime_error(error);
                }
            }
            rapidjson::Value document;
            document.SetString(doc_name.c_str(), allocator);
            documentArray.PushBack(document, allocator);
        }
    }
    return project;
}

void removeDocument(std::string &pname, Parsing::List<std::string> *doc, Storage::LinearHash<std::string> &meta) {
    std::string proj_list_key(pname);
    uint64_t doc_list = hash(proj_list_key, proj_list_key.size());
    rapidjson::Document d;
    if (meta.contains(doc_list)) {
        std::string doclist;
        meta.get(doc_list,doclist);
        d.Parse(doclist.c_str());
        rapidjson::Value &projectArray = d["__DOCUMENTS__"];
        Parsing::List<std::string> *spot = doc;
        while (spot) {
            for (rapidjson::Value::ConstValueIterator itr = projectArray.Begin(); itr != projectArray.End(); ++itr) {
                std::string dd = itr->GetString();
                std::string docname = spot->value;
                if (!dd.compare(docname)) {
                    projectArray.Erase(itr);
                    break;
                }
            }
            spot = spot->next;
        }
        meta.put(doc_list, toString(&d) );
    }
}

void addProject(std::string &pname, Storage::LinearHash<std::string> &meta) {
    std::string proj_list_key("__PROJECTS__");
    uint64_t project_list = hash(proj_list_key, proj_list_key.size());
    rapidjson::Document d;
    rapidjson::Value proj;
    proj.SetString(pname.c_str(), d.GetAllocator());

    if (meta.contains(project_list)) {
        std::string project;
        meta.get(project_list,project);
        d.Parse(project.c_str());
        rapidjson::Value &projectArray = d["__PROJECTS__"];
        projectArray.PushBack(proj, d.GetAllocator());
    } else {
        d.SetObject();
        rapidjson::Value projectArray(rapidjson::kArrayType);
        projectArray.PushBack(proj, d.GetAllocator());
        d.AddMember("__PROJECTS__", projectArray, d.GetAllocator());
    }

    meta.put( project_list, toString(&d) );
}

// Given a project remove all documents from it
void removeDocuments( std::string &pname , Storage::LinearHash<std::string> &meta ) {
    uint64_t proj_hash = hash(pname, pname.size());
    if (meta.contains(proj_hash)) {
        std::string doc_list;
        meta.get(proj_hash, doc_list);
        rapidjson::Document d2;
        d2.Parse(doc_list.c_str());
        rapidjson::Value &docArray = d2["__DOCUMENTS__"];
        for (rapidjson::Value::ConstValueIterator itr = docArray.Begin(); itr != docArray.End(); ++itr) {
            std::string dd = itr->GetString();
            std::string proj_doc_key(pname + "." + dd);
            uint64_t proj_doc_hash = hash(proj_doc_key, proj_doc_key.size());
            if (meta.contains(proj_doc_hash)) {
                meta.remove(proj_doc_hash);
            }
        }
        meta.remove(proj_hash);
    }
}

// Remove the project name from the project metadata list.
void removeProject(std::string &pname, Storage::LinearHash<std::string> &meta) {
    std::string proj_list_key("__PROJECTS__");
    uint64_t project_list = hash(proj_list_key, proj_list_key.size());
    rapidjson::Document d;
    if (meta.contains(project_list)) {
        std::string projects;
        meta.get(project_list,projects);

        d.Parse(projects.c_str());
        rapidjson::Value &projectArray = d["__PROJECTS__"];

        // TODO: map function on list?
        for (rapidjson::Value::ConstValueIterator itr = projectArray.Begin(); itr != projectArray.End(); ++itr) {
            std::string dd = itr->GetString();
            if (!dd.compare(pname)) {
                projectArray.Erase(itr);
                break;
            }
        }

        // Update data in hashtable
        meta.put( project_list , toString( &d ) );

        removeDocuments( pname , meta );
    }

}

void removeKey(std::string pname, std::string dname, std::string key, Storage::LinearHash<std::string> &meta) {
    std::string doc(pname + "." + dname);
    uint64_t doc_hash = hash(doc, doc.size());
    if (meta.contains(doc_hash)) {
        rapidjson::Document d;
        std::string doc_text;
        meta.get(doc_hash,doc_text);
        d.Parse(doc_text.c_str());
        if (d.HasMember(key.c_str())) {
            d.RemoveMember(key.c_str());
            meta.put(doc_hash,toString(&d));
        }
    }
}

uint64_t appendData(size_t length, std::string data, std::string dataFile) {
    std::ofstream fout;
    std::ifstream fin;
    fin.open(dataFile.c_str(), std::ios::in | std::ios::binary);
    fin.seekg(0, fin.end);
    int pos = fin.tellg();
    fin.close();

    fout.open(dataFile.c_str(), std::ios::app | std::ios::binary);
    assert(!fout.fail());
    fout.write(reinterpret_cast<char*>(&length), sizeof(length));
    fout.write(data.c_str(), length);
    fout.close();

    if (pos == -1)	
        return 0;
    else
        return pos;
}

// Add firleds to a document
void addFields(std::string pname, std::string dname, rapidjson::Document &d,
        Storage::LinearHash<std::string> &meta,
        Storage::LinearHash<uint64_t> &indices,
        std::string dataFile) {

    std::string key_string(pname + "." + dname);
    uint64_t key = hash(key_string, key_string.size());
    rapidjson::Document arrayContainer;
    // If the document exists, we just want to append
    rapidjson::Value uuidArray(rapidjson::kArrayType);
    if (meta.contains(key)) {
        std::string fieldArrayStr;
        meta.get(key,fieldArrayStr);
        arrayContainer.Parse(fieldArrayStr.c_str());
        uuidArray = arrayContainer["__FIELDS__"];
    } else {
        arrayContainer.SetObject();
    }
    rapidjson::Value::ConstMemberIterator itr = d.MemberBegin();
    while (itr != d.MemberEnd()) {
        rapidjson::Document kv;
        kv.SetObject();
        const std::string k_str = itr->name.GetString();
        rapidjson::Value k(k_str.c_str(), kv.GetAllocator());
        rapidjson::Value &v_tmp = d[k_str.c_str()];
        rapidjson::Value v(v_tmp, kv.GetAllocator());

        // Generate UUID
        std::string uuid = newUUID();

        // Add UUID to json array
        rapidjson::Value field;
        field.SetString(uuid.c_str(), arrayContainer.GetAllocator());
        uuidArray.PushBack(field, arrayContainer.GetAllocator());

        rapidjson::Value uuid_v;
        uuid_v.SetString(uuid.c_str(), kv.GetAllocator());
        kv.AddMember("__UUID__", uuid_v, kv.GetAllocator());
        kv.AddMember(k, v, kv.GetAllocator());
        std::string data = toString(&kv);
        size_t len = data.size();

        // Write data out to a file, record file offset and save with UUID as key
        uint64_t offset = appendData(len, data, dataFile);
        uint64_t uuid_key = hash(uuid, uuid.size());
        indices.put(uuid_key, offset);

        // Insert uuid for particular field in metadata
        std::string fieldKeyStr(pname + "." + dname + "." + k_str);
        uint64_t fieldKey = hash(fieldKeyStr, fieldKeyStr.size());

        rapidjson::Value field2;
        field2.SetString(uuid.c_str(), arrayContainer.GetAllocator());
        // Duplicates...
        if (meta.contains(fieldKey)) {
            std::string uuids;
            meta.get(fieldKey,uuids);
            rapidjson::Document uuid_doc;
            uuid_doc.Parse(uuids.c_str());
            rapidjson::Value &uuidArray = uuid_doc["__FIELDS__"];
            uuidArray.PushBack(field2, uuid_doc.GetAllocator());
            meta.put(fieldKey,toString(&uuid_doc));
        } else {
            rapidjson::Document uuid_doc;
            uuid_doc.SetObject();
            rapidjson::Value uuidArray;
            uuidArray.SetArray();
            uuidArray.PushBack(field2, uuid_doc.GetAllocator());
            uuid_doc.AddMember("__FIELDS__", uuidArray, uuid_doc.GetAllocator());
            std::string uuids = toString(&uuid_doc);
            meta.put(fieldKey, uuids);
        }
        d.RemoveMember(k_str.c_str());
    }
    rapidjson::Value name_string;
    name_string.SetString(dname.c_str(), arrayContainer.GetAllocator());
    arrayContainer.AddMember("__NAME__", name_string, arrayContainer.GetAllocator());
    arrayContainer.AddMember("__FIELDS__", uuidArray, arrayContainer.GetAllocator());
    meta.put(key, toString(&arrayContainer));
}

std::string combineFields(Parsing::List<rapidjson::Document> *fields) {
    rapidjson::Document doc;
    doc.SetObject();
    while (fields) {
        rapidjson::Document &field = fields->value;
        for (rapidjson::Value::ConstMemberIterator itr = field.MemberBegin(); itr != field.MemberEnd(); ++itr) {
            const std::string k_str = itr->name.GetString();
            rapidjson::Value k(k_str.c_str(), doc.GetAllocator());
            rapidjson::Value &v_tmp = field[k_str.c_str()];
            rapidjson::Value v(v_tmp, doc.GetAllocator());
            doc.AddMember(k, v, doc.GetAllocator());
        }
        fields = fields->next;
    }
    return toString(&doc);
}

Parsing::List<rapidjson::Document> *filterFields(Parsing::List<rapidjson::Document> *fields, Parsing::List<std::string> *keys) {

    if (keys) {
        if (!keys->value.compare("*")) return fields;
    }

    Parsing::List<rapidjson::Document> *result = NULL; 
    Parsing::List<rapidjson::Document> *result_spot = NULL; 

    while (keys) {
        if (!keys->value.compare("*")) {
            std::cout << "If selecting all fields, '*' must appear first." << std::endl;
            return NULL;
        }
        if (keys->aggregate) {
            keys = keys->next;
            continue;
        }
        Parsing::List<rapidjson::Document> *spot = fields;
        while (spot) {
            rapidjson::Document &field = spot->value;
            if (field.HasMember(keys->value.c_str())) {
                rapidjson::Value k(keys->value.c_str(), field.GetAllocator());
                rapidjson::Value &v_tmp = field[keys->value.c_str()];
                rapidjson::Value v(v_tmp, field.GetAllocator());
                if (!result_spot) {
                    result = new Parsing::List<rapidjson::Document>();
                    result->value.SetObject();
                    result->value.AddMember(k, v, result->value.GetAllocator());
                    result_spot = result;
                } else {
                    result_spot->next = new Parsing::List<rapidjson::Document>();
                    result_spot = result_spot->next;
                    result_spot->value.SetObject();
                    result_spot->value.AddMember(k, v, result->value.GetAllocator());
                }
            }
            spot = spot->next;
        }
        keys = keys->next;
    }
    return result;
}

Parsing::List<std::string> *extractAggregates(Parsing::List<std::string> *keys) {
    Parsing::List<std::string> *result = NULL;
    Parsing::List<std::string> *spot = NULL;
    while (keys) {
        if (keys->aggregate != NULL) {
            if (!spot) {
                result = new Parsing::List<std::string>();
                result->aggregate = keys->aggregate;
                result->value = keys->value;
                spot = result;
            } else {
                Parsing::List<std::string> *node = new Parsing::List<std::string>(keys->aggregate, keys->value);
                spot->append(node);
            }
        }
        keys = keys->next;
    }
    return result;
}

void processAggregates(Parsing::List<rapidjson::Document> *fields, Parsing::List<std::string> *aggs) {
    while (aggs) {
        Parsing::Aggregate &f = *(aggs->aggregate);
        std::string aggName = Parsing::Aggregates[f];
        std::string key = aggs->value;

        bool first_run = true;
        double res = 0;
        Parsing::List<rapidjson::Document> *spot = fields;
        Parsing::List<rapidjson::Document> *parent = NULL;
        while (spot) {
            rapidjson::Document &d = spot->value;
            if (!d.HasMember(key.c_str())) {
                parent = spot;
                spot = spot->next;
                continue;
            }
            rapidjson::Value &v = d[key.c_str()];
            if (!v.IsNumber()) {
                parent = spot;
                spot = spot->next;
                continue;
            }
            if (f == Parsing::SUM) {
                if (first_run) {
                    res = 0.0;
                }
                if (v.IsDouble()) {
                    res += v.GetDouble();
                } else {
                    res += v.GetInt();
                }
            } else if (f == Parsing::MIN) {
                double tmp;
                if (first_run) {
                    res = std::numeric_limits<double>::max();	
                }
                if (v.IsDouble()) {
                    tmp = v.GetDouble();
                } else {
                    tmp = v.GetInt();
                }
                if (tmp < res) {
                    res = tmp;
                }	
            } else if (f == Parsing::MAX) {
                double tmp;
                if (first_run) {
                    res = std::numeric_limits<double>::min();	
                }
                if (v.IsDouble()) {
                    tmp = v.GetDouble();
                } else {
                    tmp = v.GetInt();
                }
                if (tmp > res) {
                    res = tmp;
                }	
            }
            if (first_run) {
                first_run = false;
            }
            parent = spot;
            spot = spot->next;	
        }
        if (parent) {
            parent->next = new Parsing::List<rapidjson::Document>();
            std::string fieldName(aggName + "(" + key + ")");
            rapidjson::Value k(fieldName.c_str(), parent->next->value.GetAllocator());
            rapidjson::Value v;
            v.SetDouble(res);
            parent->next->value.SetObject();
            parent->next->value.AddMember(k, v, parent->next->value.GetAllocator());
        }
        aggs = aggs->next;
    }
}

void execute(Parsing::Query &q, Storage::LinearHash<std::string> &meta, Storage::LinearHash<uint64_t> &indices, std::string dataFile) {
    clock_t start, end;
    start = std::clock();	
    switch (q.command) {
        case Parsing::CREATE:
            // Create the project and/or document with or without any values.
            {
                // Insert the project and document if it doesn't already exist
                if (q.project) {
                    uint64_t project_key = hash(*q.project, (*q.project).size());
                    if (meta.contains(project_key)) {
                        // Project exists.  Append the document to the list
                        std::string docList;
                        meta.get(project_key,docList);
                        try {
                            rapidjson::Document proj = appendDocument(docList, q.documents);
                            std::string project_json = toString(&proj);
                            meta.put( project_key , project_json );
                        } catch (std::runtime_error &e) {
                            std::cout << e.what() << std::endl;
                            return;
                        }
                    } else {
                        // Project doesn't exist.  Create project and add document if one is being created.
                        rapidjson::Document project = createProject(*q.project, q.documents);
                        addProject( *q.project, meta);
                        std::string project_json = toString(&project);
                        meta.put(project_key, project_json);
                    }
                }
                Parsing::List<std::string> *spot = q.documents;
                while (spot) {
                    rapidjson::Document d;
                    if (q.value && !d.Parse(q.value->c_str()).HasParseError()) {
                        std::cout << toPrettyString(&d) << std::endl;
                        addFields( *q.project, spot->value, d, meta, indices, dataFile);
                    } else {
                        // No value.  Create empty documents;
                        d.SetObject();
                        rapidjson::Value docname;
                        docname.SetString(spot->value.c_str(), d.GetAllocator());
                        d.AddMember("__NAME__", docname, d.GetAllocator());
                    }
                    spot = spot->next;
                }
                break;
            }
        case Parsing::SELECT:
            {
                // Build a JSON object with the results
                Parsing::List<std::string> *keyList = q.keys;
                keyList->unique();
                Parsing::List<std::string> *aggregates = extractAggregates(keyList);
                Parsing::List<std::string> *uuidList = extractUUIDArray(*q.project, q.documents->value, keyList, meta);
                Parsing::List<rapidjson::Document> *fieldList = NULL;
                if (uuidList) {
                    fieldList = new Parsing::List<rapidjson::Document>();

                    Parsing::List<rapidjson::Document> *spot = fieldList;

                    spot->value = readData(dataFile, uuidList->value, indices);
                    uuidList = uuidList->next;

                    // Build a linked list of fields
                    while (uuidList) {
                        spot->next = new Parsing::List<rapidjson::Document>();
                        spot = spot->next;
                        spot->value = readData(dataFile, uuidList->value, indices);
                        uuidList = uuidList->next;
                    }
                }
                // Filter the fields
                Parsing::List<rapidjson::Document> *filtered = filterFields(fieldList, keyList);

                // Process aggregate functions
                processAggregates(filtered, aggregates);

                std::string result = combineFields(filtered);
                std::cout << toPrettyString(result) << std::endl;

                break;
            }
        case Parsing::DELETE:
            // Delete a project or document
            {
                Parsing::List<std::string> *spot = q.documents;
                if (spot) {
                    while (spot) {
                        std::string key(*q.project + "." + spot->value);
                        uint64_t doc_hash = hash(key, key.size());
                        if (meta.contains(doc_hash)) {
                            meta.remove(doc_hash);
                        }
                        removeDocument( *q.project, spot, meta);
                        spot = spot->next;
                    }
                } else {
                    removeProject( *q.project, meta);
                }
                break;
            }
        case Parsing::SHOW:
            {
                // List the projects
                if (!q.project->compare("__PROJECTS__")) {
                    std::string p("__PROJECTS__");
                    uint64_t phash = hash(p, p.size());
                    if (meta.contains(phash)) {
                        std::string data;
                        meta.get(phash,data);
                        std::cout << toPrettyString(data) << std::endl;
                    } else {
                        std::cout << "No projects found." << std::endl;
                    }
                } else { // List the documents in a specific project
                    uint64_t dhash = hash( *q.project, q.project->size());
                    if (meta.contains(dhash)) {
                        std::string data;
                        meta.get(dhash,data);
                        std::cout << toPrettyString(data) << std::endl;
                    } else {
                        std::cout << "No documents found." << std::endl;
                    }
                }
                break;
            }
        case Parsing::REMOVE:
            {
                // Remove a key
                if (q.documents) {
                    std::string doc = q.documents->value;
                    Parsing::List<std::string> *spot = q.keys;
                    while (spot) {
                        removeKey(*q.project, doc, spot->value, meta);
                        spot=spot->next;
                    }
                }
                break;
            }
        case Parsing::ALTER:
            // Alter a document
            q.print();
            break;
        default:
            std::cout << "Command not recognized." << std::endl;
    }
    end = std::clock();
    std::cout << "Done!  Took " << 1000 * (float)(end - start) / CLOCKS_PER_SEC << " milliseconds." << std::endl;
}

inline bool file_exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

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
    return matcher( text , state, list_index , len, Parsing::CreateArgs , LENGTH(Parsing::Commands) );
}

char *selectFromMatcher(const char *text, int state) {
    static unsigned int list_index, len;
    return matcher( text , state, list_index , len, Parsing::SelectArgs , LENGTH(Parsing::Commands) );
}


char *selectMatcher(const char *UNUSED(text), int state) {
    if (!state) {
        char *arr = (char *)malloc(5);
        strcpy(arr, "FROM");
        arr[4] = '\0';
        return arr;
    } else {
        return (char *)NULL;
    }
}

char *documentMatcher(const char *UNUSED(text), int state) {
    if (!state) {
        char *arr = (char *)malloc(11);
        strcpy(arr, "WITH VALUE");
        arr[10] = '\0';
        return arr;
    } else {
        return (char *)NULL;
    }
}

std::vector<std::string> split(const char *str, char c = ' ') {
    std::vector<std::string> result;
    do {
        const char *begin = str;
        while(*str != c && *str) {
            str++;
        }

        result.push_back(std::string(begin, str));
    } while (0 != *str++);

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
            if (tokens.size() == 3) {
                if (!strcasecmp(tokens[1].c_str(), "document")) {
                    matches = rl_completion_matches((char *)tokens.back().c_str(), &documentMatcher);
                }
            } else if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &createMatcher);
            }
        } else if (!strcasecmp(tokens[0].c_str(), "select")) {
            if (tokens.size() == 3) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &selectFromMatcher);
            } else if (tokens.size() == 2) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &selectMatcher);
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
    std::string meta_fname("meta.lht");
    std::string indices_fname("indices.lht");
    std::string data_fname("data.db");
    if (argc > 1) {
        meta_fname = argv[1];
    }
    if (argc > 2) {
        indices_fname = argv[2];
    }
    if (argc > 3) {
        data_fname = argv[3];
    }
    char *buf;
    Storage::LinearHash<std::string> *meta;
    Storage::LinearHash<uint64_t> *indices;

    if (file_exists(meta_fname)) {
        meta = readFromFile<std::string>(meta_fname);
    } else {
        meta = new Storage::LinearHash<std::string>(1025,2048);
    }

    if (file_exists(indices_fname)) {
        indices = readFromFile<uint64_t>(indices_fname);
    } else {
        indices = new Storage::LinearHash<uint64_t>(1025,2048);
    }

    rl_startup_hook = start_readline;

    std::cout << "Enter a query (q to quit):" << std::endl;
    while (1) {
        buf = readline("> ");

        if (buf == NULL) {
            break;
        }
        if (buf[0] != 0) {
            std::cout << "Adding to history: " << buf << std::endl;
            add_history(buf);
        }

        std::string q(buf);
        if (!q.compare("q")) {
            break;
        }
        Parsing::Parser p(q);
        Parsing::Query *query = p.parse();
        if (query) {
            std::cout << "Buf: " << buf << std::endl;
            execute( *query, *meta, *indices, data_fname);
        }
        std::cout << std::endl;
    }
    std::cout << "Goodbye!" << std::endl;
    free(buf);
    dumpToFile(meta_fname, *meta);
    dumpToFile(indices_fname, *indices);

    return 0;
}
