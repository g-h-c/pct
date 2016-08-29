QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pct
TEMPLATE = app

SOURCES += extractheaders\main.cpp \
maincmd\maincmd.cpp \
maingui\maingui.cpp

FORMS    += forms\mainwindow.ui 

SOURCES=extractheaders\main.cpp

