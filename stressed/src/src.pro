TEMPLATE = app

CONFIG += qt warn_on

TARGET = stressed

QMAKE_CLEAN += $(TARGET)

RESOURCES +=  ../resources/resources.qrc

FORMS   += mainwindow.ui \
           textresource.ui

HEADERS += mainwindow.h \
           resource.h \
           settings.h \
           stunpack.h \
           textresource.h

SOURCES += main.cpp \
           mainwindow.cpp \
           resource.cpp \
           settings.cpp \
           stunpack.c \
           textresource.cpp
