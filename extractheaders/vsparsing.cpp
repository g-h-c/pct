#include "vsparsing.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <regex>
#include <fstream>

using namespace tinyxml2;
using namespace std;

VcxprojParsing::VcxprojParsing(const char* path)
{				
	if (doc.LoadFile(path) != XMLError::XML_SUCCESS)
		throw runtime_error(string("Cannot open: ") + path + ": " + doc.ErrorName());
	

}

void VcxprojParsing::parse(vector<ProjectConfiguration>& configurations,
	                  vector<string>& files)
{
	XMLElement* project = project = doc.FirstChildElement("Project");;
	XMLElement* itemGroup = project->FirstChildElement("ItemGroup");
	XMLElement* itemDefinitionGroup = project->FirstChildElement("ItemDefinitionGroup");

	while (itemGroup) {
		const char* labelItemGroup;
		XMLElement* clCompile;

		if ((labelItemGroup = itemGroup->Attribute("Label")) && !strcmp(labelItemGroup, "ProjectConfigurations")) {
			XMLElement* projectConfiguration = itemGroup->FirstChildElement("ProjectConfiguration");

			while (projectConfiguration) {
				const char* label = projectConfiguration->Attribute("Include");
				const char* configurationName = projectConfiguration->FirstChildElement("Configuration")->GetText();

				configurations.push_back({label, configurationName});
				projectConfiguration = projectConfiguration->NextSiblingElement("ProjectConfiguration");
			}
		}

		clCompile = itemGroup->FirstChildElement("ClCompile");

		while (clCompile) {
			files.push_back(clCompile->Attribute("Include"));
			clCompile = clCompile->NextSiblingElement("ClCompile");
		}
		
		itemGroup = itemGroup->NextSiblingElement("ItemGroup");
	}
	
	while (itemDefinitionGroup) {
		for (auto& configuration : configurations) {			
			const char* label = itemDefinitionGroup->Attribute("Condition");

			if (label == "'$(Configuration)|$(Platform)'=='" + configuration.name + "'") {
				XMLElement* clCompile = itemDefinitionGroup->FirstChildElement("ClCompile");
				XMLElement* definitions = clCompile? clCompile->FirstChildElement("PreprocessorDefinitions") : NULL;
				XMLElement* includeDirs = clCompile ? clCompile->FirstChildElement("AdditionalIncludeDirectories") : NULL;
				XMLElement* precompiledHeaderFile = clCompile ? clCompile->FirstChildElement("PrecompiledHeaderFile") : NULL;

				if (definitions) {
					configuration.definitions = definitions->FirstChild()->ToText()->Value();

					// replace things like %(PreprocessorDefinitions) which extract headers cannot understand
					configuration.definitions = regex_replace(configuration.definitions, regex("%\\(.*\\)"), string(""));
					
				}

				if (includeDirs) 
					configuration.additionalIncludeDirectories = includeDirs->FirstChild()->ToText()->Value();

				if (precompiledHeaderFile)
					configuration.precompiledHeaderFile = precompiledHeaderFile->FirstChild()->ToText()->Value();
				
			}
			itemDefinitionGroup = itemDefinitionGroup->NextSiblingElement("ItemDefinitionGroup");
		}
	}	


}

SlnParsing::SlnParsing(const char* path)
{
	ifstream file(path);
	string line;

	while (getline(file, line)) {
		if (!line.empty())
			fileContents.push_back(line);
	}
}

void SlnParsing::parse(std::vector<Project>& projects)
{	
		for (auto& line : fileContents) {
			std::regex rgx(R"%(Project\("\{.?.?.?.?.?.?.?.?-.?.?.?.?-.?.?.?.?-.?.?.?.?-.?.?.?.?.?.?.?.?.?.?.?.?\}"\)\s*=\s*"(.*)"\s*,\s*"(.*)"\s*,\s*"\{.?.?.?.?.?.?.?.?-.?.?.?.?-.?.?.?.?-.?.?.?.?-.?.?.?.?.?.?.?.?.?.?.?.?\}\s*")%");
			std::smatch match;

			regex_search(line, match, rgx);

			if (!match.empty() && match.length() >= 2) {
				Project project;

				project.name = match[1];
				project.location = match[2];
				projects.push_back(project);
			}
		}
}



