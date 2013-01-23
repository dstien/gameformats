TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += opengl widgets

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += shaperesource.ui

HEADERS += flagdelegate.h \
           materialdelegate.h \
           materialsmodel.h \
           shapemodel.h \
           shaperesource.h \
           shapeview.h \
           typedelegate.h \
           types.h \
           verticesmodel.h

SOURCES += flagdelegate.cpp \
           materialdelegate.cpp \
           materialsmodel.cpp \
           shapemodel.cpp \
           shaperesource.cpp \
           shapeview.cpp \
           typedelegate.cpp \
           verticesmodel.cpp
