"# pct"

Analyses C/C++ file to generate a precompiled header. The precompiled header will consist of the standard headers that are included in the file (or any header included by the file recursively). It uses Boost Wave Preprocessor to do so.

Usage example:

--sysinclude "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" --sysinclude C:\Qt\Qt5.6.0\5.6\msvc2013_64\include --sysinclude C:\Qt\Qt5.6.0\5.6\msvc2013_64\include\QtCore --def "_WIN32;WIN32;_M_X64_" --input c:\path\to\file.cpp --include c:\path\to\ 2>nul

(2>nul discards STDERR)

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







