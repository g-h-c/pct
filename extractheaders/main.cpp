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
// system or thirdparty headers
set<string> systemheaders;
vector<string> inputs;
vector<string> includedirs;
vector<string> excludedirs;
vector<string> sysincludedirs;
vector<string> includeheaders;
vector<string> excludeheaders;
// contains the headers inclusion, as they were found
// in the file. This is how they will be copied to the generated
// precompiled header, to keep relatives paths, case, etc.
vector<string> headersfound;
int nesting;
string cxxflags;

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
		if (find(includeheaders.begin(), includeheaders.end(), relname) != includeheaders.end())
			systemheaders.insert(relname);
		else {
			if (!is_system_include)
				userheadersqueue.push(relname);
			else {
				path file_path(relname);
				string filename = strtolower(string(file_path.filename().string()));
				

					if (find(excludeheaders.begin(), excludeheaders.end(), filename) == excludeheaders.end() &&
						!subpath(excludedirs, absname))
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
            "Files to parse in search of standard/thirdparty includes, separated by semicolon (as that is how CMake returns lists")
        ("include,I", po::value<vector<string> >(&includedirs)->composing(),
            "specify an additional include directory")
        /*("exclude,E", po::value<vector<string> >(&excludedirs)->composing(),
            "specify a directory which files will be not included in the precompiled header (nor its subfolders, recursively)")*/
        ("excludeheader", po::value<vector<string> >(&excludeheaders)->composing(), 
            "specify a header file that will not be included in the precompiled header. This option is case insensitive.")
        ("includeheader", po::value<vector<string> >(&includeheaders)->composing(), 
            "specify a user header that will be included in the precompiled header, even if it was in a system or thirdparty include path")
        ("sysinclude,S", po::value<vector<string> >(&sysincludedirs)->composing(),
            "specify an additional system or thirdparty include directory")
        ("nesting,n", po::value<int>(&nesting)->default_value(0),
            "specify a new maximal include nesting depth")        
        ("def,D", po::value<string>(&cxxflags),
            "macros to be definited. E.g. --def _M_X64")
#if BOOST_WAVE_SUPPORT_PRAGMA_ONCE != 0
        ("noguard,G", "disable include guard detection")
#endif
        // it may interesting to be able to specify a directory as input from user 
        // and also to allow an option -r to do it recursively
    ;
	
    parsed_options parsed = command_line_parser(argc, argv).options(desc_options).run();
	
    try {
		store(parsed, vm);
	} catch (std::exception& e) {
		cerr << "Error parsing command line: " << e.what() << endl;
	}

    notify(vm);  

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
    for (auto dir : sysincludedirs)  {
        ctx.add_sysinclude_path(dir.c_str());
    }
}

void add_user_includes(context_type& ctx)
{
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

    instream.unsetf(std::ios::skipws);
    instr = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
        std::istreambuf_iterator<char>());    
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
    ctx.set_max_include_nesting_depth(nesting); // to avoid wave to exit prematurely on iostream.h
	
    add_macro_definitions(ctx, cxxflags);
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
                << e.description() << std::endl;                        
        }
        catch (boost::wave::cpp_exception const& e) {
            std::string filename = e.file_name();
            cerr
                << filename << "(" << e.line_no() << "): "
                << e.description() << std::endl;                
        }            
    } while (!is_end);                            
}

void write_stdafx()
{
    
    cout << "#ifndef STDAFX_H\n";
    cout << "#define STDAFX_H\n";
    
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
    readOptions(argc, argv);
        
    for (auto input : inputs) {
        vector<string> inputList;
        splitInput(inputList, input);

        for (auto filename : inputList) {
            userheadersqueue.push(filename);
        }
    }
    
    while (!userheadersqueue.empty()) {
        string header = userheadersqueue.front();
        process_file(header);
        userheadersqueue.pop();
    }
    write_stdafx();

    return EXIT_SUCCESS;
}
