#ifndef VSPARSING_H
#define VSPARSING_H
#include <string>
#include <map>
#include <vector>
#include "tinyxml2.h"

struct ProjectConfiguration {
	std::string name;
	std::string configuration;
	std::string definitions;
	std::string additionalIncludeDirectories;
	std::string precompiledHeaderFile;
};

struct Project {
	std::string name;
	std::string location;
};

class VcxprojParsing {
public:

	// @path path to a .vcxproj
	// @throw std::runtime_error If the file could not be opened
	VcxprojParsing(const char* path);
	void parse(std::vector<ProjectConfiguration>& configurations,
			   std::vector<std::string>& files);	

private:

	tinyxml2::XMLDocument doc;

};

class SlnParsing {
public:

	// @path path to a .vcxproj
	// @throw std::runtime_error If the file could not be opened
	SlnParsing(const char* path);
	void parse(std::vector<Project>& projects);

private:
	std::vector<std::string> fileContents;
};

#endif
