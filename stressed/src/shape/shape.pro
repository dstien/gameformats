TEMPLATE = lib

CONFIG += staticlib warn_on qt

QT += opengl

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

FORMS   += shaperesource.ui

HEADERS += materialsmodel.h \
           matrix.h \
           shapemodel.h \
           shaperesource.h \
           shapeview.h \
           typedelegate.h \
           verticesmodel.h

SOURCES += materialsmodel.cpp \
           matrix.cpp \
           shapemodel.cpp \
           shaperesource.cpp \
           shapeview.cpp \
           typedelegate.cpp \
           verticesmodel.cpp
