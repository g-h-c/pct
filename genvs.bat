rem Run this on x64 Native tools command prompt

%QTDIR%\bin\qmake -r -spec win32-msvc2013 -tp vc QMAKE_CXXFLAGS+=/MP CONFIG+="%*" pct.pro


