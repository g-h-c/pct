#include "extractheaders.h"


#include <boost/wave/language_support.hpp>


#include <boost/filesystem/operations.hpp>
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <string>
#include <queue>
#include <set>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <functional>

using namespace std;
using namespace boost;
using namespace wave;
using namespace boost::wave;

using namespace boost::filesystem;

typedef boost::wave::cpplexer::lex_token<> token_type;

typedef boost::wave::cpplexer::lex_iterator<token_type> lexer_type;



template <typename Token>
class find_includes_hooks
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

		if (find(impl->input.includeheaders.begin(), impl->input.includeheaders.end(), relname) != impl->input.includeheaders.end())
			impl->systemheaders.insert(relname);
		else {
			if (find(impl->input.excludeheaders.begin(), impl->input.excludeheaders.end(), filename) == impl->input.excludeheaders.end()) {
				if (!is_system_include)
					impl->userheadersqueue.push(boost::filesystem::canonical(absname));
				else
					if (!subpath(impl->input.excludedirs, absname))
						impl->systemheaders.insert(relname);
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
		impl->output.headersfound.push_back(filename);
		return false;
	}

	ExtractHeadersImpl* impl = NULL;
};

typedef boost::wave::context<
	std::string::const_iterator, lexer_type,
	boost::wave::iteration_context_policies::load_file_to_string,
	find_includes_hooks<token_type> > context_type;


class ExtractHeadersImpl {
public:
	ExtractHeadersImpl(ExtractHeadersOutput& out, const ExtractHeadersInput& in);	
	void process_file(const path& filename);
	void add_macro_definitions(context_type& context, const std::string& cxx_flags);
	void add_system_includes(context_type& ctx);
	void add_user_includes(context_type& ctx);
	void write_stdafx();
	void run();

	std::queue<path> userheadersqueue;
	std::set<path> headersprocessed;
	std::set<path> includedirs;
	std::vector<string> sysincludedirs;
	// system or thirdparty headers
	std::set<boost::filesystem::path> systemheaders;	

	const ExtractHeadersInput& input;
	ExtractHeadersOutput& output;
};


ExtractHeaders::~ExtractHeaders()
{
}

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


ExtractHeaders::ExtractHeaders()

{	
}

ExtractHeadersImpl::ExtractHeadersImpl(ExtractHeadersOutput& out, const ExtractHeadersInput& in) :
	input(in),
	output(out)
{
	
	for (const auto& includeDir : input.includedirsIn)
	{
		includedirs.insert(boost::filesystem::canonical(includeDir));
	}

	for (const auto& includeDir : input.sysincludedirs)
	{
		sysincludedirs.push_back(includeDir);
	}

	
}

void ExtractHeaders::write_stdafx()
{
	assert(impl && "run has not been called yet");
	impl->write_stdafx();
}

void ExtractHeaders::run(ExtractHeadersOutput& output, const ExtractHeadersInput& input)
{
	impl.reset(new ExtractHeadersImpl(output, input));
	impl->run();
}

void ExtractHeadersImpl::add_macro_definitions(context_type& context, const string& cxx_flags)
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


void ExtractHeadersImpl::add_system_includes(context_type& ctx)
{
	for (auto& sysdir : input.sysincludetreedirs) {

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

void ExtractHeadersImpl::add_user_includes(context_type& ctx)
{
	for (auto& userdir : input.includetreedirs) {

		includedirs.insert(boost::filesystem::canonical(userdir));

		if (is_directory(userdir)) {
			vector<string> dirs = getAllDirsInDir(userdir.c_str());

			for (auto& dir : dirs) {

				includedirs.insert(boost::filesystem::canonical(dir));
		}
		}
	}

	for (auto dir : includedirs)  {
		ctx.add_include_path(dir.string().c_str());
	}
}

void ExtractHeadersImpl::process_file(const path& filename)
{
//  create the wave::context object and initialize it from the file to
//  preprocess (may contain options inside of special comments)

	std::ifstream instream(filename.string().c_str());
	string instr;
	context_type::iterator_type it;
	context_type::iterator_type end;
	bool is_end = false;
	string cxxflagsstr;
	find_includes_hooks <token_type> hooks;

	hooks.impl = this;
	instream.unsetf(std::ios::skipws);
	instr = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
		std::istreambuf_iterator<char>());    

	if (input.verbose) 
		output.infoStream << "Preprocessing input file: " << filename.generic_string()
			              << "..." << std::endl;

    context_type ctx(instr.begin(),
		             instr.end(),
					 filename.string().c_str(),
					hooks);

	//  add special predefined macros

	// TODO GerardoHernandez configure this in the command line
	ctx.set_language(language_support(support_cpp11 |
		                              support_option_variadics |
									  support_option_long_long |
									  support_option_include_guard_detection 
									 ));
	// it is best not to go too deep, headers like <iostream> on windows
	// give problem with boost wave
	ctx.set_max_include_nesting_depth(input.nesting);
	
	for (auto& def : input.cxxflags) {
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
			output.errorStream
					<< filename << "(" << e.line_no() << "): "
					<< "Lexical error: " << e.description() << std::endl;      
			break;
	} catch (boost::wave::cpp_exception const& e) {
			if (e.get_errorcode() != preprocess_exception::include_nesting_too_deep) {
				std::string filename = e.file_name();
				output.errorStream
					<< filename << "(" << e.line_no() << "): "
					<< e.description() << std::endl;
			}
	}
	} while (!is_end);
}

void ExtractHeadersImpl::write_stdafx()
{
	path outputpath(input.outputfile);
	string guardname = outputpath.filename().string();
	size_t dotpos = guardname.find_first_of(".");
	
	guardname = guardname.substr(0, dotpos);

	for (auto & c : guardname)
		c = toupper(c);
		
	if (input.pragma) 
	output.outputStream << "#pragma once\n\n";    
	else {
		output.outputStream << "#ifndef " + guardname + "_H\n";
		output.outputStream << "#define " + guardname + "_H\n";
	}

	for (auto header : systemheaders) {
		string headername = header.filename().string();
		auto header_it = output.headersfound.begin();

		while (header_it != output.headersfound.end()) {
			if (header_it->find(headername) != string::npos) {
				output.outputStream << "#include " << *header_it << "\n";
				output.headersfound.erase(header_it);
				break;
			}
			header_it++;
		}
	}

	if (!input.pragma) 
		output.outputStream << "#endif\n";
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


// @throws runtime_error if an input path does no exist
void ExtractHeadersImpl::run()
{
	for (auto& input : input.inputs) {
		vector<string> inputList;
		splitInput(inputList, input);

		for (auto& input_path : inputList) {

			if (is_directory(input_path)) {
				vector<string> files = getAllFilesInDir(input_path.c_str());

				for (auto& file : files) {
					userheadersqueue.push(boost::filesystem::canonical(file));
				}
			}
			else if (!exists(input_path))
				throw runtime_error("Cannot find: " + input_path + "\n");
			else
				userheadersqueue.push(boost::filesystem::canonical(input));
		}
	}

	output.errorStream << "Processing " << userheadersqueue.size() << " reported includes" << std::endl;
	while (!userheadersqueue.empty()) {
		path header = userheadersqueue.front();
		if (headersprocessed.count(header) == 0) {
			process_file(header);
			headersprocessed.insert(header);
		}

		userheadersqueue.pop();
	}		
}

std::string& strtolower(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);

	return str;
}
