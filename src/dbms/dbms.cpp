#include <fstream> 
#include <sstream> 
#include <limits>
#include <cstddef>

#include "dbms.h"
#include "../storage/LinearHash.h"
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <pretty.h>
#include <UUID.h>

std::string createRow(const rapidjson::Value &doc, const std::string key, std::string docUUID, std::string fieldUUID) {
	rapidjson::Document d;
	d.SetObject();

	// Insert the data.
	rapidjson::Value k(key.c_str(), d.GetAllocator());
	rapidjson::Value v(doc[key.c_str()], d.GetAllocator());
	d.AddMember(k, v, d.GetAllocator());

	rapidjson::Value fieldID(fieldUUID.c_str(), d.GetAllocator());
	d.AddMember("_field", fieldID, d.GetAllocator());

	rapidjson::Value dID(docUUID.c_str(), d.GetAllocator());
	d.AddMember("_doc", dID, d.GetAllocator());

	return toString(&d);
}

void insertDocument(const rapidjson::Value &doc, std::string docUUID, std::string projUUID, INDICES &indices, META &meta) {
	assert(doc.GetType() == rapidjson::kObjectType);
	rapidjson::Document d;
	d.SetObject();
	for (rapidjson::Value::ConstMemberIterator itr = doc.MemberBegin(); itr < doc.MemberEnd(); ++itr) {
		// Create a serialized row of data with some metadata fields
		std::string fieldUUID = newUUID();
		std::string row = createRow(doc, itr->name.GetString(), docUUID, fieldUUID);
		// Insert this row into the DB
		//TODO: Filesystem stuff
		std::cout << toPrettyString(row) << std::endl;
	}
}

void insertDocuments(rapidjson::Document &docs, std::string pname, INDICES &indices, META &meta) {
	uint64_t projectHash = hash(pname, pname.size());
	if (!meta.contains(projectHash)) {
		// project doesn't exist.  Create a UUID for it.
		std::string proj_uuid = newUUID();
		meta.put(projectHash, proj_uuid);
		
		uint64_t uuidHash = hash(proj_uuid, proj_uuid.size());
		std::string emptyArray("[]");
		meta.put(uuidHash, emptyArray);
	}

	std::string projectUUID;
	meta.get(projectHash, projectUUID);

	// If it't an array of documents.  Iterate over them and insert each document.
	if (docs.GetType() == rapidjson::kArrayType) {
		for (rapidjson::SizeType i = 0; i < docs.Size(); ++i) {
			insertDocument(docs[i], newUUID(), projectUUID, indices, meta);
		}
	} else if (docs.GetType() == rapidjson::kObjectType) {
		insertDocument(docs, newUUID(), projectUUID, indices, meta);
	}
	
}

void execute(Parsing::Query &q, META &meta, INDICES &indices, std::string dataFile) {
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
		insertDocuments(docs, project, indices, meta);
		break;
	    }
        case Parsing::SELECT:
            {
		q.print();
                break;
            }
        case Parsing::DELETE:
            // Delete a project or document
            {
		q.print();
                break;
            }
        case Parsing::SHOW:
            {
		q.print();
                break;
            }
        case Parsing::UPDATE:
            {
		q.print();
                break;
            }
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
                matches = rl_completion_matches((char *)tokens.back().c_str(), &createMatcher);
        } else if (!strcasecmp(tokens[0].c_str(), "select")) {
                matches = rl_completion_matches((char *)tokens.back().c_str(), &selectMatcher);
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
        //indices = readFromFile<uint64_t>(indices_fname);
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
