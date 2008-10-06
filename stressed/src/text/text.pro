TEMPLATE = lib

CONFIG += staticlib warn_on qt

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += textresource.ui

HEADERS += textresource.h

SOURCES += textresource.cpp
