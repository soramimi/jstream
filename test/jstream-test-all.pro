
TARGET = test
TEMPLATE = app

INCLUDEPATH += ../include
win32:INCLUDEPATH += C:/googletest-1.17.0/googletest/include

LIBS += -lgtest
win32:CONFIG(debug,debug|release):LIBS += -LC:/googletest-1.17.0/build/lib/Debug
win32:CONFIG(release,debug|release):LIBS += -LC:/googletest-1.17.0/build/lib/Release

gcc:QMAKE_CXXFLAGS += -Wall -Wextra -Werror=return-type -Werror=trigraphs -Wno-switch -Wno-reorder -Wno-unused-parameter -Wno-unused-parameter

HEADERS += \
    test.h \
	../include/jstream.h
SOURCES += \
    test.cpp \
    test1.cpp \
    test2.cpp \
    test3.cpp \
    test4.cpp \
    testmain.cpp
