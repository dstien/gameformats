TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += widgets

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += speedresource.ui

HEADERS += speedresource.h

SOURCES += speedresource.cpp
