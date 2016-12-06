# pct [![AppVeyor build](https://ci.appveyor.com/api/projects/status/uiue2rplcirf0a38?svg=true)](https://ci.appveyor.com/project/g-h-c/pct/)

Pct (PreCompiled header tool) aims to be a bag of tools to help reducing and analysing C/C++ compilation times. There is only one tool for now, extractheaders.

##extractheaders

Analyses C / C++ files to generate a precompiled header. It can get its input from Visual Studio project files (.vcxproj and .sln) or they can be specified explicitly through command line options. The precompiled header will consist of the standard headers that are included in the provided files (or any header included by the files recursively). It uses Boost Wave Preprocessor under the hood.

extractheaders can read Visual Studio project files with the options --sln and --vcxproj. E.g.

--sln c:\\path\\to\\mysolution.sln --sysinclude "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\include" --configuration "Debug|x64"

This command line will parse all the .vcxproj in mysolution.sln, and will generate the precompiled headers according to the macros and include paths specified in the configuration Debug|x64

If you do not have Visual Studio project files, you can specifiy which are your inputs like this:

--sysinclude "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\include" --sysinclude C:\\Qt\\Qt5.6.0\\5.6\\msvc2013_64\\include --sysinclude C:\\Qt\\Qt5.6.0\\5.6\\msvc2013_64\\include\\QtCore --def "_WIN32;WIN32;_M_X64_" --input c:\\path\\to\\file.cpp --include c:\\path\\to\\ 

Applied to this file.cpp:

```cpp
#include <vector>
#include <QtCore/QAtomicInt>
#include <quuid.h>
#include "myinclude.h"

int main()
{
  // lots of interesting code here
}
```

where myinclude.h is:

```cpp
#include <array>
#include <iostream>
```

will generate this precompiled header:

```cpp
#ifndef STDAFX_H
#define STDAFX_H
#include <array>
#include <iostream>
#include <vector>
#include <QtCore/QAtomicInt>
#include <quuid.h>
#endif
```


extractheaders can also be easily integrated with qmake. Imagine the previous example was built with this .pro file:

```cpp
HEADERS += myinclude.h
SOURCES += file.cpp
INCLUDEPATH += "."

for (it, SOURCES) {    
	INPUT_ARGUMENTS += --input \"$${it}\"
}

for (it, INCLUDEPATH) {    
	INCLUDE_ARGUMENTS += --include \"$$PWD/$${it}\"
}

system(extractheaders --sysinclude \"c:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\include\\" --sysincludetree C:\\Qt\\Qt5.6.0\\5.6\\msvc2013_64\\include --def "_WIN32;WIN32;_M_X64;_IOSTREAM_" $$INPUT_ARGUMENTS $$INCLUDE_ARGUMENTS) --excluderegexp "moc_.*"
```

system() will invoke extractheaders in this case, generating the appropiate stdafx.h. The first two loops will generate the necessary arguments that the tool needs. The option --sysincludetree comes in handy to include all the Qt include subfolders.

The option --excluderegexp "moc_.*" avoids processing Qt moc-generated files, which otherwise will generate a lot of errors.

It may also be easier to generate Visual Studio project files with qmake and then tell extractheaders to parse them: https://cppisland.wordpress.com/2015/11/15/cross-platform-development-with-c/

**Compilation**

Requires boost libraries and C++ 11 compliant compiler. The environment variable BOOST_HOME needs to be set to point to the root of Boost libraries and BOOST_LIB needs to point the binaries. The least Boost version I tried was 1.58.0. I have only worked on 64 bits, a 32 bits build should work but I have not tested it. Althought the Visual Studio project files were generated with version 2015, the code compiles cleanly on 2013 as well (but you may need to select ```Visual Studio 2013 (v120)``` as Platform Toolset in Configuration Properties->General.

Both qmake and Visual Studio project files are provided. The script genvs.bat could be used to generate the Visual Studio project files from the qmake ones.

**Blog entry**

https://cppisland.wordpress.com/2016/06/05/introducing-pct-a-tool-to-help-reducing-cc-compilation-times/

**Troubleshooting**

> error: could not find include file: windows.h

Add the path to Windows SDK user mode header files, for instance with: --sysinclude C:\Program Files (x86)\Windows Kits\8.1\Include\um

> Could not find include file even if it is one of the specified include directories

This may be caused by trying to include a system header using quotes, e.g. #include "string.h" or trying to include a user header using angle brackets, e.g. #include \<myinclude.h\>. Best solution would be to be consistent with the normal convention, use quotes for user headers and angle bracket for system headers.

> error: ill formed preprocessor directive: #include "aheader.h"

This may be cause because one the preprocessed files does not include an end-of-line at the end of the file.

> error: Windows Kits\8.1\Include\shared\ws2def.h(221): error C2011: 'sockaddr' : 'struct' type redefinition

winsock2.h and windows.h are sensitive to the order in which they are included. winsock2.h must be included before windows.h or you can #define WIN32_LEAN_AND_MEAN if you do not need rarely used windows.h stuff





