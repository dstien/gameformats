TEMPLATE = app

CONFIG += qt warn_on

TARGET = stressed

QMAKE_CLEAN += $(TARGET)

RESOURCES +=  ../resources/resources.qrc

FORMS   += bitmapresource.ui \
           mainwindow.ui \
           textresource.ui

HEADERS += bitmapresource.h \
           mainwindow.h \
           resource.h \
           settings.h \
           stunpack.h \
           textresource.h

SOURCES += bitmapresource.cpp \
           main.cpp \
           mainwindow.cpp \
           resource.cpp \
           settings.cpp \
           stunpack.c \
           textresource.cpp
