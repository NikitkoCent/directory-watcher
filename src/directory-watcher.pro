
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = directory-watcher
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

CONFIG += c++14


SOURCES += \
    main.cpp \
    view/mainwindow.cpp \
    model/directory_watcher_worker.cpp \
    model/change_entry.cpp \
    model/windows/ci_char_traits.cpp \
    model/windows/directory_watcher.cpp \
    model/windows/file_operations.cpp \
    model/windows/hashes.cpp \
    model/windows/path.cpp \
    controller/mainwindow_controller.cpp

HEADERS += \
    view/mainwindow.h \
    model/directory_watcher.h \
    model/file_operations.h \
    model/ordered_set.h \
    model/path.h \
    model/directory_watcher_worker.h \
    model/qt_directory_watcher_worker.h \
    model/windows/win_extend_path_limit.h \

FORMS += view/mainwindow.ui
