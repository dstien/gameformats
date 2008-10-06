TEMPLATE = lib

CONFIG += staticlib warn_on qt

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += bitmapresource.ui

HEADERS += bitmapresource.h

SOURCES += bitmapresource.cpp
