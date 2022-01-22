QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += testcase
CONFIG += no_testcase_installs
CONFIG += file_copies

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    testparser.cpp \
    testscanner.cpp

SOURCES += $$files(../app/*.cpp)
SOURCES -= ../app/main.cpp

# message("My sources: " + $$SOURCES)

HEADERS += \
    testparser.h \
    testscanner.h

HEADERS += $$files(../app/*.h)

INCLUDEPATH += ../app/
INCLUDEPATH += $$OUT_PWD/../app/

wc3reforgedscripts.files = $$files(../../wc3reforged/*.j)
wc3reforgedscripts.path = $$OUT_PWD/wc3reforged

COPIES += wc3reforgedscripts
