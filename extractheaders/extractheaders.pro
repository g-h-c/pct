QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = extractheaderslib
TEMPLATE = lib
CONFIG += staticlib

HEADERS += tinyxml2.h \
vsparsing.h \
extractheaders.h

SOURCES += extractheaders.cpp \
tinyxml2.cpp \
vsparsing.cpp

INCLUDEPATH += $(BOOST_HOME)
LIBS += -L$(BOOST_HOME)/stage/lib


