TARGET = jstream
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$PWD

INCLUDEPATH += include

SOURCES += main.cpp test/test.cpp

HEADERS += \
	include/jstream.h

