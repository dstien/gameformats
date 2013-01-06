TEMPLATE = lib

CONFIG += staticlib warn_on qt

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += rawresource.ui

HEADERS += rawresource.h

SOURCES += rawresource.cpp
