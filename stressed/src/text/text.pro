TEMPLATE = lib

CONFIG += staticlib warn_on qt

QMAKE_CLEAN += $(TARGET)

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += textresource.ui

HEADERS += textresource.h

SOURCES += textresource.cpp
