#-------------------------------------------------
#
# Project created by QtCreator 2021-01-12T15:40:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtest
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
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/../include/
DEPENDPATH += $$PWD/../include/

INCLUDEPATH += $$PWD/../include/ortp/
DEPENDPATH += $$PWD/../include/ortp/

INCLUDEPATH += $$PWD/../include/mediastreamer2/
DEPENDPATH += $$PWD/../include/mediastreamer2/

INCLUDEPATH += $$PWD/../include/bctoolbox/
DEPENDPATH += $$PWD/../include/bctoolbox/

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lbctoolbox
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lbctoolbox
else:unix: LIBS += -L$$PWD/../lib/ -lbctoolbox

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lliblinphone
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lliblinphone
else:unix: LIBS += -L$$PWD/../lib/ -lliblinphone

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lortp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lortp
else:unix: LIBS += -L$$PWD/../lib/ -lortp

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lmediastreamer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lmediastreamer
else:unix: LIBS += -L$$PWD/../lib/ -lmediastreamer

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
