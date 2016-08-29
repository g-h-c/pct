#ifndef EXTRACTHEADERS_H
#define EXTRACTHEADERS_H
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <memory>

struct ExtractHeadersInput {
	std::vector<std::string> inputs;
	std::vector<std::string> includedirsIn;
	std::vector<std::string> includetreedirs;
	std::vector<std::string> excludeheaders;
	std::vector<std::string> includeheaders;
	std::vector<std::string> sysincludedirs;
	std::vector<std::string> sysincludetreedirs;
	std::vector<std::string> cxxflags;
	std::string vcxproj;
	std::string configuration;
	// TODO g-h-c
	std::vector<std::string> excludedirs;
	
	int nesting;
	std::string outputfile;
	bool verbose;
	bool pragma;
};

struct ExtractHeadersOutput {
	ExtractHeadersOutput(std::ostream& errStream,
						std::ostream& infStream,
						std::ostream& outStream) :
		errorStream(errStream),
		infoStream(infStream),
		outputStream(outStream)
	{
	}

	// headersfound contains the headers inclusion, as they were found
	// in the file. This is how they will be copied to the generated
	// precompiled header, to keep relatives paths, case, etc.
	std::vector<std::string> headersfound;
	std::ostream& errorStream;
	std::ostream& infoStream;
	std::ostream& outputStream;
};

class ExtractHeadersImpl;

class ExtractHeaders {
public:
	ExtractHeaders();
	~ExtractHeaders();
	void write_stdafx();	
	void run(ExtractHeadersOutput& output, const ExtractHeadersInput& input);

private:
	std::unique_ptr<ExtractHeadersImpl> impl;
};

std::string& strtolower(std::string& str);

#endif

