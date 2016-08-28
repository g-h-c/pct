QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
  
TARGET = extractheadersgui
TEMPLATE = app
CONFIG+=console
HEADERS+= mainwindow.h
SOURCES += extractheadersgui.cpp \
           mainwindow.cpp
FORMS+=../forms/extractheaders.ui
INCLUDEPATH += $(BOOST_HOME)
               INCLUDEPATH += ../extractheaders

CONFIG(debug, debug|release): CONF_FOLDER=debug
CONFIG(release, debug|release): CONF_FOLDER=release
LIBS += -L$(BOOST_HOME)/stage/lib
LIBS += -L../extractheaders/$$CONF_FOLDER -lextractheaders


