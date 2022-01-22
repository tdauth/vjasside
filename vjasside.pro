QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += testcase
CONFIG += no_testcase_installs

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autocompletionpopup.cpp \
    main.cpp \
    mainwindow.cpp \
    testparser.cpp \
    testscanner.cpp \
    vjassast.cpp \
    vjassfunction.cpp \
    vjassfunctionparameter.cpp \
    vjasskeyword.cpp \
    vjassparseerror.cpp \
    vjassparser.cpp \
    vjassparseresult.cpp \
    vjassscanner.cpp \
    vjasstoken.cpp \
    vjasstype.cpp

HEADERS += \
    autocompletionpopup.h \
    mainwindow.h \
    testparser.h \
    testscanner.h \
    vjassast.h \
    vjassfunction.h \
    vjassfunctionparameter.h \
    vjasskeyword.h \
    vjassparseerror.h \
    vjassparser.h \
    vjassparseresult.h \
    vjassscanner.h \
    vjasstoken.h \
    vjasstype.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md
