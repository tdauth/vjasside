QT       += core

CONFIG += c++11
CONFIG += no_testcase_installs
CONFIG += console

SOURCES += $$files(../app/*.cpp)
SOURCES -= ../app/main.cpp
SOURCES -= ../app/mainwindow.cpp
SOURCES -= ../app/autocompletionpopup.cpp
SOURCES -= ../app/linenumbers.cpp
SOURCES -= ../app/highlightinfo.cpp

# message("My sources: " + $$SOURCES)

HEADERS += $$files(../app/*.h)
HEADERS -= ../app/mainwindow.h
HEADERS -= ../app/autocompletionpopup.h
HEADERS -= ../app/linenumbers.h
HEADERS -= ../app/highlightinfo.h

SOURCES += \
    main.cpp

# message("My sources: " + $$SOURCES)

INCLUDEPATH += ../app/
INCLUDEPATH += $$OUT_PWD/../app/
