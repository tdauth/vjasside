QT       += core

CONFIG += c++11
CONFIG += no_testcase_installs
CONFIG += console

# Remove all classes requiring QtGui or QtWidgets

SOURCES += $$files(../app/*.cpp)
SOURCES -= ../app/main.cpp
SOURCES -= ../app/mainwindow.cpp
SOURCES -= ../app/autocompletionpopup.cpp
SOURCES -= ../app/linenumbers.cpp
SOURCES -= ../app/highlightinfo.cpp
SOURCES -= ../app/textedit.cpp
SOURCES -= ../app/syntaxhighlighter.cpp
SOURCES -= ../app/finddialog.cpp

# message("My sources: " + $$SOURCES)

HEADERS += $$files(../app/*.h)
HEADERS -= ../app/mainwindow.h
HEADERS -= ../app/autocompletionpopup.h
HEADERS -= ../app/linenumbers.h
HEADERS -= ../app/highlightinfo.h
HEADERS -= ../app/textedit.h
HEADERS -= ../app/syntaxhighlighter.h
HEADERS -= ../app/finddialog.h

SOURCES += \
    main.cpp

# message("My sources: " + $$SOURCES)

INCLUDEPATH += ../app/
INCLUDEPATH += $$OUT_PWD/../app/
