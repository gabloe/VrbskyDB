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
	if (doc) {
		rapidjson::Value document;
		document.SetString(doc->value.c_str(), allocator);
		documentArray.PushBack(document, allocator);
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

void execute(Parsing::Query &q, Storage::LinearHash<std::string> &projects, Storage::LinearHash<std::string> &documents) {
	switch (q.command) {
	case Parsing::CREATE:
		// Create the project and/or document with or without any values.
		{
			// Insert the project and document if it doesn't already exist
			if (q.project) {
				uint64_t project_key = hash(*q.project, (*q.project).size());
				if (projects.contains(project_key)) {
					// Project exists.  Append the document to the list
					std::string *docList = projects.get(project_key);
					try {
						rapidjson::Document proj = appendDocument(docList, q.documents);
						projects.remove(project_key);
						std::string project_json = toString(&proj);
						projects.put(project_key, new std::string(project_json));
					} catch (std::runtime_error &e) {
						std::cout << e.what() << std::endl;
						return;
					}
				} else {
					// Project doesn't exist.  Create project and add document if one is being created.
					rapidjson::Document project = createProject(*q.project, q.documents);
					std::string project_json = toString(&project);
					projects.put(project_key, new std::string(project_json));
				}
				std::cout << "\nInserted Project:" << std::endl;
				std::cout << *projects.get(project_key) << std::endl;
			}
			rapidjson::Document d;
			if(q.value && q.documents) {
				std::string project_doc(*q.project + "." + q.documents->value);
				uint64_t document_key = hash(project_doc, project_doc.size());
				if (!d.Parse(q.value->c_str()).HasParseError()) {
					if (!d.HasMember("__NAME__")) {
						rapidjson::Value docname;
						docname.SetString(q.documents->value.c_str(), d.GetAllocator());
						d.AddMember("__NAME__", docname, d.GetAllocator());
					}
					std::string value = toString(&d);
					documents.put(document_key, new std::string(value));
				} else {
					std::cout << "Could not parse JSON." << std::endl;
				}
				std::cout << "Inserted Document:" << std::endl;
				std::cout << *documents.get(document_key) << std::endl;
			}
		}
		break;
	case Parsing::SELECT:
		// Build a JSON object with the results
		break;
	case Parsing::DELETE:
		// Delete a project or document
		break;
	case Parsing::REMOVE:
		// Remove a key
		break;
	case Parsing::APPEND:
		// Append to a field
		break;
	default:
		std::cout << "Command not recognized." << std::endl;
	}
}

int main(void) {
	std::string q = "";
	Storage::LinearHash<std::string> projects(1024, 2048);
	Storage::LinearHash<std::string> documents(1024, 2048);
	while (1) {
		std::cout << "Enter a query (q to quit):" << std::endl;
		getline(std::cin, q);
		if (!q.compare("q")) {
			break;
		}
		Parsing::Parser p(q);
		Parsing::Query *query = p.parse();
		if (query) {
			execute(*query, projects, documents);
		}
		std::cout << std::endl;
	}
	return 0;
}
