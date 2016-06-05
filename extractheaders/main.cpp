#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/language_support.hpp>
#include <boost/wave.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include <queue>
#include <set>
#include <iostream>
#include <stdexcept>
#include <boost/program_options.hpp>
#include <sstream>

using namespace std;
using namespace boost;
using namespace wave;
using namespace boost::wave;
using namespace boost::program_options;
using namespace boost::filesystem;
namespace po = boost::program_options;

boost::program_options::options_description desc_options;
boost::program_options::variables_map vm;
queue<string> userheadersqueue;
set<string> headersprocessed;

// system or thirdparty headers
set<string> systemheaders;
vector<string> inputs;
vector<string> includedirs;
vector<string> includetreedirs;
vector<string> excludedirs;
vector<string> sysincludedirs;
vector<string> sysincludetreedirs;
vector<string> includeheaders;
vector<string> excludeheaders;
// contains the headers inclusion, as they were found
// in the file. This is how they will be copied to the generated
// precompiled header, to keep relatives paths, case, etc.
vector<string> headersfound;
int nesting;
vector<string> cxxflags;
string outputfile;

string& strtolower(string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);

	return str;
}

template <typename Token>
class print_opened_include_files
	: public boost::wave::context_policies::eat_whitespace<Token>
{
public:
	template <typename Context>
	void
		opened_include_file(Context const& ctx, std::string relname,
		std::string absname, bool is_system_include)
	{
		path file_path(relname);
		string filename = strtolower(string(file_path.filename().string()));

		if (find(includeheaders.begin(), includeheaders.end(), relname) != includeheaders.end())
			systemheaders.insert(relname);
		else {
			if (find(excludeheaders.begin(), excludeheaders.end(), filename) == excludeheaders.end()) {
				if (!is_system_include)
					userheadersqueue.push(relname);
				else
					if (!subpath(excludedirs, absname))
						systemheaders.insert(relname);
			}
		}		
	}

	template <typename Context>
	void
		detected_include_guard(Context const& ctx, std::string const& filename,
		std::string const& include_guard)
	{		
	}

	template <typename Context>
	bool found_include_directive(Context const& ctx,
		std::string const &filename, bool include_next)
	{		
		headersfound.push_back(filename);
		return false;
	}

};


typedef boost::wave::cpplexer::lex_token<> token_type;
typedef boost::wave::cpplexer::lex_iterator<token_type> lexer_type;
typedef boost::wave::context<
        std::string::const_iterator, lexer_type,
        boost::wave::iteration_context_policies::load_file_to_string,
        print_opened_include_files<token_type> > context_type;

bool iscplusplusfile(path filepath)
{
    const char* extensions[] =
    { ".cpp",
    ".cxx",
    ".c",
    ".cc",
    NULL
    };
    const string file_ext = filepath.extension().string();
    unsigned int pos = 0;

    while (extensions[pos]) {
        if (file_ext == extensions[pos])
            return true;

        pos++;
    }

    return false;
}

// gets all files in a dir (non-recursively)
vector<string> getAllFilesInDir(const char* dir)
{
    path path(dir);
    directory_iterator end_iter;
    vector<string> result;

    if (exists(path) && is_directory(path))
    {
        for (directory_iterator dir_iter(path); dir_iter != end_iter; ++dir_iter)
        {
            if (is_regular_file(dir_iter->status()) &&
                iscplusplusfile(dir_iter->path()))
                result.push_back(dir_iter->path().string());
        }
    }

    return result;
}

// gets all dirs in a dir (non-recursively)
vector<string> getAllDirsInDir(const char* dir)
{
    path path(dir);
    directory_iterator end_iter;
    vector<string> result;

    if (exists(path) && is_directory(path))
    {
        for (directory_iterator dir_iter(path); dir_iter != end_iter; ++dir_iter)
        {
            if (is_directory(dir_iter->status()))
                result.push_back(dir_iter->path().string());
        }
    }

    return result;
}


bool subpath(const vector<string> paths, const string& path)
{
    return false;
    /*// TODO GerardoHernandez
    assert(false);
    return false;*/
}


void readOptions(int argc, char** argv)
{
    desc_options.add_options()
        ("input", po::value<vector<string> >(&inputs)->composing(),
         "Files/directory to parse in search of standard/thirdparty includes. In case of directories only .c .cc .cpp .cxx files will be parsed (and the headers included in those)"
         "If a directory is specified, all the files of that directory will be parsed. More than one file can be specified if separated by semicolons"
         "(this option could be specified multiple times)")
        ("include,I", po::value<vector<string> >(&includedirs)->composing(),
         "specify an additional include directory")
        ("includetree,I", po::value<vector<string> >(&includetreedirs)->composing(),
         "same as --include, but the subfolders of the specified folder are searched as well (non-recursively, i.e. only the first level of subfolders)")
        /*("exclude,E", po::value<vector<string> >(&excludedirs)->composing(),
            "specify a directory which files will be not included in the precompiled header (nor its subfolders, recursively)")*/
        ("excludeheader", po::value<vector<string> >(&excludeheaders)->composing(),
         "specify a header file that will not be included in the precompiled header, nor it will be processed. This option is case insensitive.")
        ("includeheader", po::value<vector<string> >(&includeheaders)->composing(),
         "specify a user header that will be included in the precompiled header, even if it was in a system or thirdparty include path")
        ("sysinclude,S", po::value<vector<string> >(&sysincludedirs)->composing(),
         "specify an additional system or thirdparty include directory")
        ("sysincludetree,S", po::value<vector<string> >(&sysincludetreedirs)->composing(),
         "same as --sysinclude, but the subfolders of the specified folder are searched as well (non-recursively, i.e. only the first level of subfolders). Useful with some frameworks like Qt")
		("nesting,n", po::value<int>(&nesting)->default_value(0),
		 "specify maximal include nesting depth (normally should be 0)")
		("def,D", po::value<vector<string>>(&cxxflags)->composing(),
		 "macros to be definited. Separated by semicolon E.g. --def _M_X64;_WIN32;WIN32")
		("pragma", "If specified, #pragma once will be added to the output, instead of the include guards")
		 ("output,o", po::value<string>(&outputfile)->default_value("stdafx.h"),
		  "output file")
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
	} catch (std::exception& e) {
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

	for (auto& header : excludeheaders) {
		strtolower(header);
	}
}

void add_macro_definitions(context_type& context, const string& cxx_flags)
{
    string definition;

	if (!cxx_flags.empty()) {
		for (auto elem : cxx_flags) {
			if (elem == ';') {
				context.add_macro_definition(definition, false);
				definition.clear();
			}
			else
				definition += elem;
		}

		context.add_macro_definition(definition, false);
	}
}


void add_system_includes(context_type& ctx)
{
    for (auto& sysdir : sysincludetreedirs) {

		sysincludedirs.push_back(sysdir);

        if (is_directory(sysdir)) {
            vector<string> dirs = getAllDirsInDir(sysdir.c_str());

            for (auto& dir : dirs) {

                sysincludedirs.push_back(dir);
            }
        }        		
    }

    for (auto dir : sysincludedirs)  {
        ctx.add_sysinclude_path(dir.c_str());
    }
}

void add_user_includes(context_type& ctx)
{
    for (auto& userdir : includetreedirs) {

		includedirs.push_back(userdir);

        if (is_directory(userdir)) {
            vector<string> dirs = getAllDirsInDir(userdir.c_str());

            for (auto& dir : dirs) {

                includedirs.push_back(dir);
            }
        }
    }

     for (auto dir : includedirs)  {
        ctx.add_include_path(dir.c_str());
    }
}

void process_file(const string& filename)
{
//  create the wave::context object and initialize it from the file to
//  preprocess (may contain options inside of special comments)
    
    std::ifstream instream(filename.c_str());
    string instr;
    context_type::iterator_type it;
    context_type::iterator_type end;
    bool is_end = false;
	string cxxflagsstr;

    instream.unsetf(std::ios::skipws);
    instr = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
        std::istreambuf_iterator<char>());    

	if (vm.count("verbose") > 0) 
		std::cerr << "Preprocessing input file: " << filename
			      << "..." << std::endl;
	
    context_type ctx(instr.begin(), instr.end(), filename.c_str(),
                     print_opened_include_files <token_type>());
       
    //  add special predefined macros

    // TODO GerardoHernandez configure this in the command line
	ctx.set_language(language_support(support_cpp11 |
		                              support_option_variadics |
									  support_option_long_long |
									  support_option_include_guard_detection 
									 ));
	// it is best not to go too deep, headers like <iostream> on windows
	// give problem with boost wave
    ctx.set_max_include_nesting_depth(nesting); 
	
	for (auto& def : cxxflags) {
		if (cxxflagsstr.empty())
			cxxflagsstr += def;
		else
			cxxflagsstr += ";" + def;
	}

    add_macro_definitions(ctx, cxxflagsstr);
    add_system_includes(ctx);
    add_user_includes(ctx);

    //  preprocess the input, loop over all generated tokens collecting the
    //  generated text
    it = ctx.begin();
    end = ctx.end();
           
    // perform actual preprocessing
    do  
    {
        using namespace boost::wave;
           
        try {
            ++it;
            // operator != could also throw an exception
            is_end = it != end;
        } catch (boost::wave::cpplexer::lexing_exception const& e) {
                    
            std::string filename = e.file_name();
            cerr
                << filename << "(" << e.line_no() << "): "
                << "Lexical error: " << e.description() << std::endl;      
			break;
        }
        catch (boost::wave::cpp_exception const& e) {
			if (e.get_errorcode() != preprocess_exception::include_nesting_too_deep) {
				std::string filename = e.file_name();
				cerr
					<< filename << "(" << e.line_no() << "): "
					<< e.description() << std::endl;
			}
        }            
    } while (!is_end);                            
}

void write_stdafx()
{
	path outputpath(outputfile);
	string guardname = outputpath.filename().string();
	size_t dotpos = guardname.find_first_of(".");
	
	guardname = guardname.substr(0, dotpos);

	for (auto & c : guardname)
		c = toupper(c);
		
	if (vm.count("pragma") > 0) 
        cout << "#pragma once\n\n";    
	else {
		cout << "#ifndef " + guardname + "_H\n";
		cout << "#define " + guardname + "_H\n";
	}    

    for (auto header : systemheaders) {
        string headername = path(header).filename().string();
		auto header_it = headersfound.begin();

		while (header_it != headersfound.end()) {
			if (header_it->find(headername) != string::npos) {
				cout << "#include " << *header_it << "\n";
				headersfound.erase(header_it);
				break;
			}
			header_it++;
		}        
    }

	if (vm.count("pragma") == 0) 
        cout << "#endif\n";
}

void splitInput(vector<string>& files, const string& filesstr)
{
    string file;

	if (!filesstr.empty()) {
		for (auto elem : filesstr) {
			if (elem == ';') {
				files.push_back(file);
				file.clear();
			}
			else
				file += elem;
		}

		files.push_back(file);
	}
}


int main(int argc, char** argv)
{       
	ofstream out;		

    readOptions(argc, argv);
	
	if (vm.count("verbose") > 0) {
		cout << "Arguments: " << endl;
		for (int arg = 0; arg < argc; arg++) {
			cout << argv[arg] << " ";
		}
		cout << endl;
	}

	out = ofstream(outputfile);

	if (!out.is_open()) {
		cerr << "Cannot open: " << outputfile;
	    exit(EXIT_FAILURE);
    }

	cout.rdbuf(out.rdbuf());

    for (auto& input : inputs) {
        vector<string> inputList;
        splitInput(inputList, input);

        for (auto& input_path : inputList) {

			if (is_directory(input_path)) {
				vector<string> files = getAllFilesInDir(input_path.c_str());

				for (auto& file : files) {

					userheadersqueue.push(file);
				}
			} else if (!exists(input_path)) {
				cerr << "Cannot find: " << input_path << "\n";
				exit(EXIT_FAILURE);
			}
			else
                userheadersqueue.push(input);
        }
    }
    
    while (!userheadersqueue.empty()) {
        string header = userheadersqueue.front();

		if (headersprocessed.find(header) == headersprocessed.end()) {
			process_file(header);
			headersprocessed.insert(header);
		}

        userheadersqueue.pop();
    }
    write_stdafx();

    return EXIT_SUCCESS;
}
