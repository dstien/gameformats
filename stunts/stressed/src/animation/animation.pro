TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += widgets

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += animationresource.ui

HEADERS += animationresource.h

SOURCES += animationresource.cpp
