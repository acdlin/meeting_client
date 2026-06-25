QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audioinput.cpp \
    audiooutput.cpp \
    chatmessage.cpp \
    logqueue.cpp \
    main.cpp \
    mytcpsocket.cpp \
    myvideosurface.cpp \
    netheader.cpp \
    partner.cpp \
    sendimg.cpp \
    videocell.cpp \
    widget.cpp \
    writeworker.cpp

HEADERS += \
    audioinput.h \
    audiooutput.h \
    blockqueue.h \
    chatmessage.h \
    logqueue.h \
    mytcpsocket.h \
    myvideosurface.h \
    netheader.h \
    partner.h \
    sendimg.h \
    videocell.h \
    widget.h \
    writeworker.h

FORMS += \
    widget.ui

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
