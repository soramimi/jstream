TARGET = jstream
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$PWD

SOURCES += main.cpp \
	test.cpp

HEADERS += \
	jstream.h \
	test.h

