QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += sql widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bookmark.cpp \
    bookmarkmanager.cpp \
  #  filemap.cpp \
    main.cpp \
    mainwindow.cpp \
    qfilebuffer.cpp \
    ruler.cpp \
    rulerview.cpp \
   # sqliteworker.cpp \
    timeconvertor.cpp

HEADERS += \
    bookmark.h \
    bookmarkmanager.h \
    common.h \
  #  filemap.h \
    mainwindow.h \
    qfilebuffer.h \
    ruler.h \
    rulerview.h \
    sqliteworker.h \
    timeconvertor.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += "$$IN_PWD/sqlite3/include"

CONFIG(release, debug|release) {
  win32:LIBS += -L"$$IN_PWD/sqlite3/lib/" -lsqlite3
  unix:LIBS += -L"$$IN_PWD/sqlite3/lib/" -lsqlite3
}
CONFIG(debug, debug|release) {
  win32:LIBS += -L"$$IN_PWD/sqlite3/lib/" -lsqlite3
  unix:LIBS += -L"$$IN_PWD/sqlite3/lib/" -lsqlite3
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
