TEMPLATE = app
TARGET = pod

QT        += phonon\
             network

HEADERS   += mainwindow.h \
             httpaccess.h
SOURCES   += main.cpp \
             mainwindow.cpp \
             httpaccess.cpp

RESOURCES = images/setting.qrc


# install

OTHER_FILES +=

FORMS += \
    mainwindow.ui

CONFIG += debug

CONFIG(release, debug|release) {
     release: DEFINES += NDEBUG USER_NO_DEBUG _DISABLE_LOG_
}

