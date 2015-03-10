#include <stdexcept>

#include "../storage/LinearHash.h"
#include "../parsing/Parser.h"
#include "../parsing/Scanner.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

void execute(Parsing::Query &, Storage::LinearHash<std::string> &);

// Convert a JSON object to a std::string
std::string toString(rapidjson::Document *doc) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc->Accept(writer);
	std::string str = buffer.GetString();
	return str;
}

// Create a JSON object containing a project name and list of documents (one document initially)
rapidjson::Document createProject(std::string &name, Parsing::List *doc) {
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
	project.AddMember("documents", documentArray, allocator);
	return project;
}

// Given a JSON string, parse the JSON object and append a document name to the array.
rapidjson::Document appendDocument(std::string *value, Parsing::List *doc) {
	rapidjson::Document project;
	project.Parse(value->c_str());
	rapidjson::Document::AllocatorType& allocator = project.GetAllocator();
	if (project.HasMember("documents")) {
		rapidjson::Value &documentArray = project["documents"];
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

void addProject(std::string *pname, Storage::LinearHash<std::string> &table) {
	std::string proj_list_key("__PROJECTS__");
	uint64_t project_list = hash(proj_list_key, proj_list_key.size());
	rapidjson::Document d;
	rapidjson::Value proj;
	proj.SetString(pname->c_str(), d.GetAllocator());
	if (table.contains(project_list)) {
		std::string *project = table.get(project_list);
		d.Parse(project->c_str());
		rapidjson::Value &projectArray = d["projects"];
		projectArray.PushBack(proj, d.GetAllocator());
		*project = toString(&d);
	} else {
		d.SetObject();
		rapidjson::Value projectArray(rapidjson::kArrayType);
		projectArray.PushBack(proj, d.GetAllocator());
		d.AddMember("projects", projectArray, d.GetAllocator());
		table.put(project_list, new std::string(toString(&d)));
	}
}

void removeProject(std::string *pname, Storage::LinearHash<std::string> &table) {
	std::string proj_list_key("__PROJECTS__");
	uint64_t project_list = hash(proj_list_key, proj_list_key.size());
	rapidjson::Document d;
	rapidjson::Value proj;
	proj.SetString(pname->c_str(), d.GetAllocator());
	if (table.contains(project_list)) {
		std::string *project = table.get(project_list);
		d.Parse(project->c_str());
		rapidjson::Value &projectArray = d["projects"];

		for (rapidjson::Value::ConstValueIterator itr = projectArray.Begin(); itr != projectArray.End(); ++itr) {
			std::string d = itr->GetString();
			if (!d.compare(*pname)) {
				projectArray.Erase(itr);
				break;
			}
		}

		projectArray.PushBack(proj, d.GetAllocator());
		*project = toString(&d);
	}
}

void execute(Parsing::Query &q, Storage::LinearHash<std::string> &table) {
	switch (q.command) {
	case Parsing::CREATE:
		// Create the project and/or document with or without any values.
		{
			// Insert the project and document if it doesn't already exist
			if (q.project) {
				uint64_t project_key = hash(*q.project, (*q.project).size());
				if (table.contains(project_key)) {
					// Project exists.  Append the document to the list
					std::string *docList = table.get(project_key);
					try {
						rapidjson::Document proj = appendDocument(docList, q.documents);
						std::string project_json = toString(&proj);
						*docList = project_json;
					} catch (std::runtime_error &e) {
						std::cout << e.what() << std::endl;
						return;
					}
				} else {
					// Project doesn't exist.  Create project and add document if one is being created.
					rapidjson::Document project = createProject(*q.project, q.documents);
					addProject(q.project, table);
					std::string project_json = toString(&project);
					table.put(project_key, new std::string(project_json));
				}
			}
			Parsing::List *spot = q.documents;
			while (spot) {
				rapidjson::Document d;
				std::string project_doc(*q.project + "." + spot->value);
				uint64_t document_key = hash(project_doc, project_doc.size());
				if (q.value && !d.Parse(q.value->c_str()).HasParseError()) {
					if (!d.HasMember("__NAME__")) {
						rapidjson::Value docname;
						docname.SetString(spot->value.c_str(), d.GetAllocator());
						d.AddMember("__NAME__", docname, d.GetAllocator());
					}
				} else {
					// No value.  Create empty documents;
					d.SetObject();
					rapidjson::Value docname;
					docname.SetString(spot->value.c_str(), d.GetAllocator());
					d.AddMember("__NAME__", docname, d.GetAllocator());
				}
				table.put(document_key, new std::string(toString(&d)));
				spot = spot->next;
			}
			break;
		}
	case Parsing::SELECT:
		// Build a JSON object with the results
		q.print();
		break;
	case Parsing::DELETE:
		// Delete a project or document
		{
			Parsing::List *spot = q.documents;
			if (spot) {
				while (spot) {
					std::string key(*q.project + "." + spot->value);
					uint64_t doc_hash = hash(key, key.size());
					table.remove(doc_hash);
					spot = spot->next;
				}
			} else {
				std::string key(*q.project);
				uint64_t proj_hash = hash(key, key.size());
				table.remove(proj_hash);
				removeProject(q.project, table);
			}
			break;
		}
	case Parsing::REMOVE:
		// Remove a key
		q.print();
		break;
	case Parsing::APPEND:
		// Append to a field
		q.print();
		break;
	default:
		std::cout << "Command not recognized." << std::endl;
	}
}

inline bool file_exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

int main(int argc, char **argv) {
	std::string fname = "dump.db";
	if (argc > 1) {
		fname = argv[1];
	}
	std::string q = "";
	Storage::LinearHash<std::string> *table;
	if (file_exists(fname)) {
		table = readFromFile(fname);
	} else {
		table = new Storage::LinearHash<std::string>(1025,2048);
	}
	while (1) {
		std::cout << "Enter a query (q to quit):" << std::endl;
		getline(std::cin, q);
		if (!q.compare("q")) {
			break;
		}
		Parsing::Parser p(q);
		Parsing::Query *query = p.parse();
		if (query) {
			execute(*query, *table);
		}
		std::cout << std::endl;
	}
	dumpToFile(fname, *table);
	return 0;
}
