QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += testcase
CONFIG += no_testcase_installs
CONFIG += file_copies

SOURCES += $$files(../../app/*.cpp)
SOURCES -= ../../app/main.cpp

# message("My sources: " + $$SOURCES)

HEADERS += $$files(../../app/*.h)

SOURCES += \
    testpjass.cpp

# message("My sources: " + $$SOURCES)

HEADERS += \
    testpjass.h

COPIES += wc3reforgedscripts pjass

wc3reforgedscripts.files += $$files(../../../wc3reforged/*.j) \
                            $$files(../../../wc3reforged/*.ai)
wc3reforgedscripts.path = $$OUT_PWD/wc3reforged

pjass.files += $$files(../../../pjass/*.exe)
pjass.path = $$OUT_PWD/pjass

INCLUDEPATH += ../../app/
INCLUDEPATH += $$OUT_PWD/../../app/
