TARGET = extractheaderscmd
TEMPLATE = app
CONFIG+=console
SOURCES += extractheaderscmd.cpp

INCLUDEPATH += $(BOOST_HOME)
               INCLUDEPATH += ../extractheaders

CONFIG(debug, debug|release): CONF_FOLDER=debug
CONFIG(release, debug|release): CONF_FOLDER=release
LIBS += -L$(BOOST_HOME)/stage/lib
LIBS += -L../extractheaders/$$CONF_FOLDER -lextractheaders


