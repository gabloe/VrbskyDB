
#include <iostream>
#include "Aggregator.h"
#include <pretty.h>
#include "../parsing/Parser.h"
#include <map>

Aggregator::Aggregator() {}

void Aggregator::handle(rapidjson::Value *doc, const rapidjson::Value *field, rapidjson::Document::AllocatorType &allocator) {
	rapidjson::Value &d = *doc;
	rapidjson::Value f(*field, allocator);
	std::string field_name = f["field"].GetString();
	std::string function = f["function"].GetString();
	rapidjson::Value val;
	bool remove = false;

	if (d.HasMember(field_name.c_str())) {
		rapidjson::Value v(d[field_name.c_str()], allocator);
		if (v.GetType() == rapidjson::kObjectType) {
			// check for temp
			if (v.HasMember("_temporary")) {
				val = v["_temporary"];
				remove = true;
			}
		} else {
			val = v;
		}
	}

	if (results.count(field_name)) {	
		std::vector<AggregateResult> &r = results[field_name];
		for (auto it = r.begin(); it != r.end(); ++it) {
			AggregateResult &v = *it;
			if (v.field == field_name && v.function == function) {
				v.result = process(function, v.result, &val);
				v.count++;
				break;
			}
		}
	} else {
		std::vector<AggregateResult> r;
		r.push_back(AggregateResult(field_name, function, process(&val)));
		results[field_name] = r;
	}

	if (remove) {
		doc->RemoveMember(field_name.c_str());
	}
}

AggregateResult *Aggregator::getResult(std::string field, std::string function) {
	for (auto it = results.begin(); it != results.end(); ++it) {
		std::string key = it->first;
		std::vector<AggregateResult> val = it->second;
		if (key.compare(field) == 0) {
			for (auto v = val.begin(); v != val.end(); ++v) {
				AggregateResult r = *v;
				if (r.function.compare(function) == 0) {
					if (r.function.compare("AVG") == 0) {
						r.result = r.result / r.count;
					}
					return new AggregateResult(r);
				}
			}
		}
	}
	return NULL;
} 

double Aggregator::process(rapidjson::Value *val) {
	rapidjson::Value &v = *val;
	double newVal = 0;
	if (v.IsInt()) {
		newVal = v.GetInt();
	} else if (v.IsDouble()) {
		newVal = v.GetDouble();
	}

	return newVal;
}

double Aggregator::process(std::string function, double current, rapidjson::Value *val) {
	rapidjson::Value &v = *val;

	double newVal = 0;

	if (v.IsInt()) {
		newVal = v.GetInt();
	} else if (v.IsDouble()) {
		newVal = v.GetDouble();
	}

	if (function == "SUM" || function == "AVG") {
		return current + newVal;
	} else if (function == "MIN") {
		return newVal<current?newVal:current;
	} else if (function == "MAX") {
		return newVal>current?newVal:current;
	}

	return current;
}
