
TEMPLATE = app
TARGET = test2

LIBS += -lgtest

INCLUDEPATH += ../include

win32:INCLUDEPATH += C:/googletest-1.16.0/googletest/include
win32:CONFIG(debug,debug|release):LIBS += -LC:/googletest-1.16.0/build/lib/Debug
win32:CONFIG(release,debug|release):LIBS += -LC:/googletest-1.16.0/build/lib/Release

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
