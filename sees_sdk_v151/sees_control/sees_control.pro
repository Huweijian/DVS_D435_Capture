#-------------------------------------------------
#
# Project created by QtCreator 2017-02-22T14:48:12
#
#-------------------------------------------------

QT       += core gui network widgets opengl
CONFIG += qwt

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -fno-rtti
QMAKE_CXXFLAGS_RELEASE = -O3 -DNDEBUG

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = sees_control
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Get the architecture dependent libraries directory
contains(QMAKE_HOST.arch, aarch64){
    message("Compiling for ARM v8 (aarch64)")
    LIBRARY_ARCHITECTURE = aarch64
} else:armv7l {
    message("Compiling for ARM v7 (armv7l)")
    LIBRARY_ARCHITECTURE = armv7l
} else {
    message("Compiling for generic 64bit Linux (linux64)")
    LIBRARY_ARCHITECTURE = linux64
}

LIBRARY_PATH = $${PWD}/../libiness/lib/$${LIBRARY_ARCHITECTURE}/libiness.so

!exists($${LIBRARY_PATH}) {
    warning("Library not found at $${LIBRARY_PATH}. Do you have a libiness.so for $${QMAKE_HOST.arch}?")
}

# Obtain the library versions at compile time
DEFINES += LIBINESS_VERSION=\\\"$$system(basename $(readlink --canonicalize $${LIBRARY_PATH}))\\\"

SOURCES += main.cpp\
    ui/about_dialog.cpp \
    ui/mainwindow.cpp \
    ui/rendering/renderer.cpp \
    ui/video_controls.cpp \
    ui/rendering/raw_rb_render_rule.cpp \
    ui/rendering/raw_bw_render_rule.cpp \
    ui/rendering/raw_rg_render_rule.cpp \
    ui/data_view.cpp \
    ui/plotting/plotter.cpp \
    ui/device_controls.cpp 

HEADERS  += \
    versioninfo.hpp \
    ui/about_dialog.hpp \
    ui/rendering/i_render_rule.hpp \
    ui/rendering/renderer.hpp \
    ui/mainwindow.hpp \
    ui/video_controls.hpp \
    ui/rendering/raw_rb_render_rule.hpp \
    ui/rendering/raw_bw_render_rule.hpp \
    ui/rendering/raw_rg_render_rule.hpp \
    ui/data_view.hpp \
    ui/plotting/plotter.hpp \
    ui/device_controls.hpp \
    ui/ui_strings.hpp 

FORMS    += \
    mainwindow.ui \
    about_dialog.ui \
    video_controls.ui \
    data_view.ui \
    device_controls.ui

        QMAKE_LFLAGS += "-Wl,-rpath,\\\$\$ORIGIN/../libiness/lib/$${LIBRARY_ARCHITECTURE}"

        unix:!macx: LIBS += -L../libiness/lib/$${LIBRARY_ARCHITECTURE} -liness
        unix:!macx: LIBS += -L/opt/ros/kinetic/lib/x86_64-linux-gnu -lopencv_core3
        unix:!macx: LIBS += -lboost_filesystem
        unix:!macx: LIBS += -lboost_system

INCLUDEPATH +=  /usr/local/include
INCLUDEPATH +=  /opt/ros/kinetic/include/opencv-3.3.1-dev
INCLUDEPATH += $$PWD/../libiness/include
DEPENDPATH += $$PWD/include

DISTFILES +=

RESOURCES += \
    ui/icons/icons.qrc