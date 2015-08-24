TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += widgets

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += bitmapresource.ui

HEADERS += bitmapresource.h

SOURCES += bitmapresource.cpp
