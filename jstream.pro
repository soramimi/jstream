TARGET = jstream
TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$PWD

INCLUDEPATH += include

LIBS += -lgtest

SOURCES += main.cpp test/helper.cpp

HEADERS += \
	include/jstream.h

