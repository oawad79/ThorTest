#-------------------------------------------------
#
# Project created by QtCreator 2015-08-18T07:51:10
#
#-------------------------------------------------

QT += core
CONFIG -= app_bundle
CONFIG += console
CONFIG += debug

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app
TARGET = thor
INCLUDEPATH += .
INCLUDEPATH += "/usr/local/include"

LIBS += -L"."
LIBS += -L"/usr/local/lib"

CONFIG(release, debug|release): LIBS += -lsfgui -lpugixml -lSTP -lthor -lsfml-audio -lsfml-graphics -lsfml-system -lsfml-network -lsfml-window
CONFIG(debug, debug|release): LIBS += -lsfgui -lpugixml -lSTP -lthor -lsfml-audio -lsfml-graphics -lsfml-system -lsfml-network -lsfml-window

# Input

HEADERS +=
SOURCES += main.cpp

