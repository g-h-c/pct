#include "extractheaders.h"
#include "vsparsing.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <direct.h>
#include <iostream>


boost::program_options::options_description desc_options;
boost::program_options::variables_map vm;
using namespace boost::filesystem;
using namespace boost::program_options;
namespace po = boost::program_options;
using namespace std;

void readOptions(ExtractHeadersInput& input, int argc, char** argv)
{
	

	desc_options.add_options()
		("input", po::value<vector<string> >(&input.inputs)->composing(),
		"Files/directory to parse in search of standard/thirdparty includes. In case of directories only .c .cc .cpp .cxx files will be parsed (and the headers included in those)"
		"If a directory is specified, all the files of that directory will be parsed. More than one file can be specified if separated by semicolons"
		"(this option could be specified multiple times)")
		("include,I", po::value<vector<string> >(&input.includedirsIn)->composing(),
		"specify an additional include directory")
		("includetree,I", po::value<vector<string> >(&input.includetreedirs)->composing(),
		"same as --include, but the subfolders of the specified folder are searched as well (non-recursively, i.e. only the first level of subfolders)")
		/*("exclude,E", po::value<vector<string> >(&excludedirs)->composing(),
		"specify a directory which files will be not included in the precompiled header (nor its subfolders, recursively)")*/
		("excludeheader", po::value<vector<string> >(&input.excludeheaders)->composing(),
		"specify a header file that will not be included in the precompiled header, nor it will be processed. This option is case insensitive.")
		("includeheader", po::value<vector<string> >(&input.includeheaders)->composing(),
		"specify a user header that will be included in the precompiled header, even if it was in a user include path")
		("sysinclude,S", po::value<vector<string> >(&input.sysincludedirs)->composing(),
		"specify an additional system or thirdparty include directory")
		("sysincludetree,S", po::value<vector<string> >(&input.sysincludetreedirs)->composing(),
		"same as --sysinclude, but the subfolders of the specified folder are searched as well (non-recursively, i.e. only the first level of subfolders). Useful with some frameworks like Qt")
		("nesting,n", po::value<int>(&input.nesting)->default_value(0),
		"specify maximal include nesting depth (normally should be 0)")
		("def,D", po::value<vector<string>>(&input.cxxflags)->composing(),
		"macros to be defined. Separated by semicolon E.g. --def _M_X64;_WIN32;WIN32")
		("vcxproj", po::value<string>(&input.vcxproj),
		"The Visual Studio project file the precompiled header will be generated for. Used to get input file paths, macros, include directories and precompiled header location. This option is incompatible with sln")
		("sln", po::value<string>(&input.sln),
		"Generates precompiled headers for all the projects specified in the solution. This option is incompatible with vcxproj")
		("configuration", po::value<string>(&input.configuration),
		"When --vcxproj is defined, the configuration to read macro definitions from. e.g. Debug|x64")
		("pragma", "If specified, #pragma once will be added to the output, instead of the include guards")
		("output,o", po::value<string>(&input.outputfile)->default_value("stdafx.h"),
		"output file. This option will be ignored if the project file specified via --vcxproj or --sln already specify a precompiled header file")
		("verbose", "Verbose output")
		("help,h", "Produces this help")
		/*#if BOOST_WAVE_SUPPORT_PRAGMA_ONCE != 0
		("noguard,G", "disable include guard detection")
		#endif*/
		// it may interesting to be able to specify a directory as input from user 
		// and also to allow an option -r to do it recursively
		;

	try {
		parsed_options parsed = command_line_parser(argc, argv).options(desc_options).run();
		store(parsed, vm);
	}
	catch (std::exception& e) {
		cerr << "Error parsing command line: " << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	notify(vm);

	if (vm.count("help") > 0) {
		stringstream help_stream;

		help_stream << "Analyses C / C++ file to generate a precompiled header. The precompiled header will consist of the standard headers that are included in the provided files (or any header included by the files recursively)." << endl;
		help_stream << desc_options;
		cout << help_stream.str();
		exit(EXIT_SUCCESS);
	}

	for (auto& header : input.excludeheaders) {
		strtolower(header);
	}
		
	input.verbose = vm.count("verbose") > 0;
	input.pragma = vm.count("pragma") > 0;
}

int main(int argc, char** argv)
{
	ofstream out;
	ExtractHeadersInput input;	
	ExtractHeadersConsoleOutput output(cerr, cout);
	ExtractHeaders extractHeaders;
	string outputfile;

	readOptions(input, argc, argv);

	if (input.verbose) {
		cout << "Arguments: " << endl;
		for (int arg = 0; arg < argc; arg++) {
			cout << argv[arg] << " ";
		}
		cout << endl;
	}
	
	if (!input.vcxproj.empty() && !input.sln.empty()) {
		cerr << "Cannot specify options --vcxproj and --sln at the same time";
		exit(EXIT_FAILURE);
	}
		
	try {
		if (input.sln.empty()) {
			extractHeaders.run(output, input);
			extractHeaders.write_stdafx();
		}
		else {
			vector<Project> projects;
			SlnParsing parsing(input.sln.c_str());
			path sln_path(input.sln);
			path absolute_path = canonical(sln_path).remove_filename();

			parsing.parse(projects);

			for (auto& project : projects) {
				if (_chdir(absolute_path.string().c_str()) == -1)
					output.errorStream << "Cannot chdir to directory: "
					<< sln_path.remove_filename().string()
					<< std::endl;

				input.vcxproj = project.location;
				extractHeaders.run(output, input);




				extractHeaders.write_stdafx();
			}
		}
	} catch (std::exception& e) {
		cerr << e.what();
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
