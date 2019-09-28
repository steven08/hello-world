#-------------------------------------------------
#
# Project created by QtCreator 2017-03-20T18:54:40
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyTcpServer
TEMPLATE = app


SOURCES += main.cpp\
        modbus_common.cpp \
        modbus_rtu.cpp \
        tcpserver.cpp

HEADERS  += \
    modbus_common.h \
    modbus_rtu.h \
    tcpserver.h
win32:LIBS += -lWS2_32
FORMS    += \
    tcpserver.ui
