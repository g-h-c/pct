#include "vsparsing.h"
#include "tinyxml2.h"

using namespace tinyxml2;

VsParsing::VsParsing(const char* path)
{
	XMLDocument doc;
	XMLElement* project;
	XMLElement* itemGroup;
	

	doc.LoadFile(path);
	project = doc.FirstChildElement("Project");
	itemGroup = project->FirstChildElement("ItemGroup");

	while (true) {
		XMLElement* clCompile;
		
		if (clCompile = itemGroup->FirstChildElement("ClCompile")) {
			const char* source = clCompile->Attribute("Include");
			int i;

			i = i;
		}
		itemGroup = itemGroup->NextSiblingElement("ItemGroup");
	}
		//->FirstChildElement("TITLE")->GetText();

}
