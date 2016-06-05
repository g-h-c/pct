"# pct"

Analyses C / C++ file to generate a precompiled header. The precompiled header will consist of the standard headers that are included in the provided files (or any header included by the files recursively). It uses Boost Wave Preprocessor to do so.

Usage example:

--sysinclude "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" --sysinclude C:\Qt\Qt5.6.0\5.6\msvc2013_64\include --sysinclude C:\Qt\Qt5.6.0\5.6\msvc2013_64\include\QtCore --def "_WIN32;WIN32;_M_X64_" --input c:\path\to\file.cpp --include c:\path\to\ 

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

extractheaders can be easily integrated with qmake. Imagine the previous example was built with this .pro file:

```cpp
HEADERS += myinclude.h
SOURCES += file.cpp
INCLUDEPATH += "."

message($$INCLUDEPATH)
   
for (it, SOURCES) {    
	INPUT_ARGUMENTS += --input \"$${it}\"
}

for (it, INCLUDEPATH) {    
	INCLUDE_ARGUMENTS += --include \"$$PWD/$${it}\"
}

system(extractheaders --sysinclude \"c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include\" --sysincludetree C:\Qt\Qt5.6.0\5.6\msvc2013_64\include --def "_WIN32;WIN32;_M_X64;_IOSTREAM_" $$INPUT_ARGUMENTS $$INCLUDE_ARGUMENTS)
```

system() will invoke extractheaders in this case, generating the appropiate stdafx.h. The first two loops will generate the necessary arguments that the tool needs.

**Troubleshooting**

> error: could not find include file: windows.h
Add the path to Windows SDK user mode header files, for instance with: --sysinclude C:\Program Files (x86)\Windows Kits\8.1\Include\um

> Could not find include file even if it is one of the specified include directories
This may be caused by trying to include a system header using quotes, e.g. #include "string.h" or trying to include a user header using angle brackets, e.g. #include <myinclude.h>. Best solution would be to be consistent with this convention.





