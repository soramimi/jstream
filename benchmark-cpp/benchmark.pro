
TEMPLATE = app
TARGET = benchmark
CONFIG += c++17

gcc:QMAKE_CXXFLAGS += -Wno-switch

HEADERS += ElapsedTimer.h \
	../include/jstream.h
SOURCES += main.cpp
