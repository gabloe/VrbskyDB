#include <rapidjson/document.h>
#include <map>
#include <vector>

struct AggregateResult {
	std::string field;
	std::string function;
	double result;
	int count;
	AggregateResult(std::string f, std::string fun, double r) : field(f), function(fun), result(r), count(1) {}
	AggregateResult(const AggregateResult &other): field(other.field), function(other.function), result(other.result), count(other.count) {}
};

class Aggregator {
public:
	Aggregator();
	void handle(rapidjson::Value*, const rapidjson::Value*, rapidjson::Document::AllocatorType&);
	AggregateResult *getResult(std::string, std::string);
private:
	std::map<std::string, std::vector<AggregateResult>> results;
	double process(std::string, double, rapidjson::Value*);
	double process(rapidjson::Value*);
};
