QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += no_testcase_installs
CONFIG += file_copies

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autocompletionpopup.cpp \
    highlightinfo.cpp \
    linenumbers.cpp \
    pjass.cpp \
    textedit.cpp \
    main.cpp \
    mainwindow.cpp \
    syntaxhighlighter.cpp \
    vjassast.cpp \
    vjassexpression.cpp \
    vjassfunction.cpp \
    vjassfunctionparameter.cpp \
    vjassglobal.cpp \
    vjassglobals.cpp \
    vjasskeyword.cpp \
    vjasslocalstatement.cpp \
    vjassnative.cpp \
    vjassparseerror.cpp \
    vjassparser.cpp \
    vjassscanner.cpp \
    vjasssetstatement.cpp \
    vjassstatement.cpp \
    vjasstoken.cpp \
    vjasstype.cpp

HEADERS += \
    autocompletionpopup.h \
    highlightinfo.h \
    linenumbers.h \
    pjass.h \
    textedit.h \
    mainwindow.h \
    syntaxhighlighter.h \
    vjassast.h \
    vjassexpression.h \
    vjassfunction.h \
    vjassfunctionparameter.h \
    vjassglobal.h \
    vjassglobals.h \
    vjasskeyword.h \
    vjasslocalstatement.h \
    vjassnative.h \
    vjassparseerror.h \
    vjassparser.h \
    vjassscanner.h \
    vjasssetstatement.h \
    vjassstatement.h \
    vjasstoken.h \
    vjasstype.h

FORMS += \
    linenumbers.ui \
    mainwindow.ui

COPIES += wc3reforgedscripts pjass

wc3reforgedscripts.files += $$files(../../wc3reforged/*.j) \
                            $$files(../../wc3reforged/*.ai)
wc3reforgedscripts.path = $$OUT_PWD/wc3reforged

pjass.files += $$files(../../pjass/*.exe)
pjass.path = $$OUT_PWD/pjass

#message("My scripts: " + $$files(../../wc3reforged/*.j) + " copied into " + $$OUT_PWD/wc3reforged)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
