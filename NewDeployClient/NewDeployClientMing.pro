TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
#LIBS += -lWS2_32
#LIBS += -lpthread

SOURCES += \
    NewDeployClient.cpp \
    md5.cpp

HEADERS += \
    targetver.h \
    md5.h \
    header.h

