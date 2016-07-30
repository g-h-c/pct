#include "vsparsing.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <regex>

using namespace tinyxml2;
using namespace std;

VsParsing::VsParsing(const char* path)
{				
	if (doc.LoadFile(path) != XMLError::XML_SUCCESS)
		throw runtime_error(string("Cannot open: ") + path + ": " + doc.ErrorName());
	

}

void VsParsing::parse(vector<ProjectConfiguration>& configurations,
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

				configurations.push_back({label});
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

				if (definitions) {
					configuration.definitions = definitions->FirstChild()->ToText()->Value();

					// replace things like %(PreprocessorDefinitions) which extract headers cannot understand
					configuration.definitions = regex_replace(configuration.definitions, regex("%\\(.*\\)"), string(""));
					
				}
			}
			itemDefinitionGroup = itemDefinitionGroup->NextSiblingElement("ItemDefinitionGroup");
		}
	}	


}

