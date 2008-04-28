TEMPLATE = lib

CONFIG += staticlib warn_on qt

QMAKE_CLEAN += $(TARGET)

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += bitmapresource.ui

HEADERS += bitmapresource.h

SOURCES += bitmapresource.cpp
