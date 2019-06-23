#-------------------------------------------------
#
# Project created by QtCreator 2018-12-26T18:25:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LineCatcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    fileview.cpp \
    scriptview.cpp \
    pagedfileviewwidget.cpp \
    indexviewwidget.cpp \
    ullspinbox.cpp \
    indexview.cpp \
    scripteditor.cpp \
    luasyntaxhighlighter.cpp \
    searchview.cpp \
    aboutdialog.cpp \
    scriptdocsdialog.cpp \
    gettingstarteddialog.cpp \
    settingsdialog.cpp

HEADERS += \
        mainwindow.h \
    fileview.h \
    scriptview.h \
    pagedfileviewwidget.h \
    indexviewwidget.h \
    signalingscrollbar.h \
    linenumberarea.h \
    ullspinbox.h \
    indexview.h \
    scripteditor.h \
    luasyntaxhighlighter.h \
    common.h \
    searchview.h \
    coreobjptr.h \
    aboutdialog.h \
    scriptdocsdialog.h \
    gettingstarteddialog.h \
    settingsdialog.h

FORMS +=


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../bin64_core/Release/ -llcCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../bin64_core/Debug/ -llcCore_d
else:unix: LIBS += -L$$PWD/../../../bin64_core/ -llcCore_d

INCLUDEPATH += $$PWD/../../core
DEPENDPATH += $$PWD/../../core

RC_ICONS = $$PWD/../../../resources/images/icon_xl.ico
