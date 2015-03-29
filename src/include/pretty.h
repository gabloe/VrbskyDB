#ifndef _PRETTY_H_
#define _PRETTY_H_

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>



inline std::string toString(rapidjson::Value *val) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    val->Accept(writer);
    std::string str = buffer.GetString();
    return str;
}

// Convert a JSON object to a std::string
inline std::string toString(rapidjson::Document *doc) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    std::string str = buffer.GetString();
    return str;
}

inline std::string toPrettyString(rapidjson::Document *doc) {
    rapidjson::StringBuffer out;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(out);
    doc->Accept(writer);
    return out.GetString();
}

inline std::string toPrettyString(std::string doc) {
    rapidjson::Document d;
    d.Parse(doc.c_str());
    return toPrettyString(&d);
}

inline std::string toPrettyString(char *doc) {
    return toPrettyString(std::string(doc));
}

#endif
