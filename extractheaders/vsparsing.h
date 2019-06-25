#ifndef VSPARSING_H
#define VSPARSING_H

#include "tinyxml2.h"
#include <ostream>
#include <string>
#include <map>
#include <vector>

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

class VcprojParsing {
public:

	// @path path to a .vcxproj
	// @throw std::runtime_error If the file could not be opened
	VcprojParsing(const char* path, std::ostream& errStream);
	void parse(std::vector<ProjectConfiguration>& configurations,
			   std::vector<std::string>& files);	
	void replaceEnvVars(std::string& paths);
private:
	void parsevcxproj(std::vector<ProjectConfiguration>& configurations,
		std::vector<std::string>& files);
	void parsevcproj(std::vector<ProjectConfiguration>& configurations,
		std::vector<std::string>& files);
	tinyxml2::XMLDocument doc;
	std::ostream& errorStream;
	bool isvcxproj = false;
};

class SlnParsing {
public:

	// @path path to a .vcxproj
	// @throw std::runtime_error If the file could not be opened
	SlnParsing(const char* path, std::ostream& errStream);
	void parse(std::vector<Project>& projects);

private:
	std::vector<std::string> fileContents;
	std::ostream& errorStream;
};

#endif
