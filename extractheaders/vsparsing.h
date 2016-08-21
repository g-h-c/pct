#ifndef VSPARSING_H
#define VSPARSING_H
#include <string>
#include <map>
#include <vector>
#include "tinyxml2.h"

struct ProjectConfiguration {
	std::string name;
	std::string definitions;
	std::string additionalIncludeDirectories;
};

class VsParsing {
public:

	// @throw std::runtime_error If the file could not be opened
	VsParsing(const char* path);
	void parse(std::vector<ProjectConfiguration>& configurations,
			std::vector<std::string>& files);
private:

	tinyxml2::XMLDocument doc;

};

#endif
