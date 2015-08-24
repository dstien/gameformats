TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += widgets

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += rawresource.ui

HEADERS += rawresource.h

SOURCES += rawresource.cpp
