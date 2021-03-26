TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        file-op.c \
        main.c \
        socket.c \
        w5500.c \
        wizchip_conf.c

DISTFILES += \
    LICENSE \
    README.md \
    hello-qt-c.pro.user

HEADERS += \
    file-op.h \
    socket.h \
    w5500.h \
    wizchip_conf.h
