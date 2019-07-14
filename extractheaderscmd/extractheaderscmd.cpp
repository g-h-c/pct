#include "extractheaders.h"
#include "vsparsing.h"
#include <future>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <direct.h>
#include <iostream>



boost::program_options::variables_map vm;
using namespace boost::filesystem;
using namespace boost::program_options;
namespace po = boost::program_options;
using namespace std;

void readOptions(ExtractHeadersInput& input, int argc, char** argv)
{
	boost::program_options::options_description desc_options("", 1000, 500);

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
		("excludedir", po::value<vector<string> >(&input.excludedirs)->composing(),
		"Specify directories which files will not be added to the precompiled header.")
		("excluderegexp", po::value<vector<string>>(&input.excluderegexp)->composing(),
		"An ECMAScript regexp defining the files which filename will be not processed. E.g.  moc_.* will allow to exclude Qt moc files, even if they are referenced by a .vcxproj or appear in a folder that was supposed to be processed")
		("includeheader", po::value<vector<string> >(&input.includeheaders)->composing(),
		"specify a user header that will be included in the precompiled header, even if it was in a user include path. Written without brackets or quotes. E.g. stdio.h")
		("sysinclude,S", po::value<vector<string> >(&input.sysincludedirs)->composing(),
		"specify an additional system or thirdparty include directory")
		("sysincludetree,S", po::value<vector<string> >(&input.sysincludetreedirs)->composing(),
		"same as --sysinclude, but the subfolders of the specified folder are searched as well (non-recursively, i.e. only the first level of subfolders). Useful with some frameworks like Qt")
		("nesting,n", po::value<int>(&input.nesting)->default_value(0),
		"specify maximal include nesting depth (normally should be 0)")
		("def,D", po::value<vector<string>>(&input.cxxflags)->composing(),
		"macros to be defined. Separated by semicolon E.g. --def _M_X64;_WIN32;WIN32")
		("vcproj", po::value<string>(&input.vcproj),
		"The Visual Studio project file the precompiled header will be generated for. Used to get input file paths, macros, include directories and precompiled header location. "
        "Note that most of the Visual Studio build macros are not supported. Example of unsupported build macro: $(VSInstallDir). This option is incompatible with --sln")
		("sln", po::value<string>(&input.sln),
		"Generates precompiled headers for all the projects specified in the solution. Note that most of the Visual Studio build macros are not supported. Example "
        "of unsupported build macro: $(VSInstallDir). This option is incompatible with --vcxproj")
		("configuration", po::value<string>(&input.configuration),
		"When --vcxproj is defined, the configuration to read macro definitions from. e.g. Debug|x64")
		("pragma", "If specified, #pragma once will be added to the output, instead of the include guards")
		("output,o", po::value<string>(&input.outputfile)->default_value("stdafx.h"),
		"output file. This option will be ignored if the project file specified via --vcxproj or --sln already specify a precompiled header file")
		("singlecore",
		"Do not use multiple threads to process the input (applies only if the --sln option was specified)")
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
	input.singlecore = vm.count("singlecore") > 0;
}

int main(int argc, char** argv)
{
	ofstream out;
	ExtractHeadersInput input;


	string outputfile;

	readOptions(input, argc, argv);

	if (input.verbose) {
		cout << "Arguments: " << endl;
		for (int arg = 0; arg < argc; arg++) {
			cout << argv[arg] << " ";
		}
		cout << endl;
	}

	if (!input.vcproj.empty() && !input.sln.empty()) {
		cerr << "Cannot specify options --vcxproj and --sln at the same time";
		exit(EXIT_FAILURE);
	}

	try {
		if (input.sln.empty()) {
			ExtractHeaders extractHeaders;
			ExtractHeadersConsoleOutput output(cerr, cout);

			extractHeaders.run(output, input);
			extractHeaders.write_stdafx();
		}
		else {
			vector<Project> projects;
			SlnParsing parsing(input.sln.c_str(), cerr);
			path sln_path(input.sln);
			path absolute_path = canonical(sln_path).remove_filename();
			vector<future<void>> futures;
			parsing.parse(projects);
			// prevents cout and cerr flushing text to the console at the same time
			mutex consoleMutex;


			for (auto& project : projects) {
				make_absolute(project.location, absolute_path);
				input.vcproj = project.location;
				futures.resize(futures.size() + 1);

				futures.back() = async(std::launch::async, [&, input](){
					stringstream outputStream;
					stringstream errStream;
					ExtractHeaders extractHeaders;
					ExtractHeadersConsoleOutput output(errStream, outputStream);

					extractHeaders.run(output, input);
					extractHeaders.write_stdafx();


					{
						lock_guard<mutex> guard(consoleMutex);

						cout << outputStream.str();
						cerr << errStream.str();
				    }
				});

				if (input.singlecore)
					futures.back().wait();
			}

			if (!input.singlecore) {
				for (auto& future : futures) {
					future.wait();
				}
			}
		}
	}
	catch (std::exception& e) {
		cerr << e.what();
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

