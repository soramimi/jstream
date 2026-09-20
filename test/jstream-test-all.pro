
TARGET = test
TEMPLATE = app

INCLUDEPATH += ../include
win32:INCLUDEPATH += C:/googletest-1.17.0/googletest/include

LIBS += -lgtest
win32:CONFIG(debug,debug|release):LIBS += -LC:/googletest-1.17.0/build/lib/Debug
win32:CONFIG(release,debug|release):LIBS += -LC:/googletest-1.17.0/build/lib/Release

gcc:QMAKE_CXXFLAGS += -Wall -Wextra -Werror=return-type -Werror=trigraphs -Wno-switch -Wno-reorder -Wno-unused-parameter -Wno-unused-parameter

HEADERS += \
	../include/jstream.h \
	helper.h
SOURCES += \
    helper.cpp \
    main.cpp \
    test1.cpp \
    test2.cpp \
    test3.cpp \
    test4.cpp \
    test5.cpp

DISTFILES += \
	../README.md \
	../README_CSharp.md \
	../README_Go.md \
	../README_ja.md
