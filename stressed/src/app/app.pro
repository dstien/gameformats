TEMPLATE = app

CONFIG += warn_on qt

TARGET = stressed

QMAKE_CLEAN += $(TARGET)

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

PRE_TARGETDEPS += ../bitmap/libbitmap.a \
                  ../text/libtext.a

LIBS += $$PRE_TARGETDEPS

RESOURCES +=  ../../resources/resources.qrc

FORMS   += mainwindow.ui \

HEADERS += mainwindow.h \
           resource.h \
           settings.h \
           stunpack.h

SOURCES += main.cpp \
           mainwindow.cpp \
           resource.cpp \
           settings.cpp \
           stunpack.c
