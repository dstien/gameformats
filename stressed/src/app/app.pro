TEMPLATE = app

CONFIG += warn_on qt

QT += opengl

TARGET = stressed

QMAKE_CLEAN += $(TARGET)

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

PRE_TARGETDEPS += ../bitmap/libbitmap.a \
                  ../shape/libshape.a \
                  ../text/libtext.a

LIBS += $$PRE_TARGETDEPS

RESOURCES +=  ../../resources/resources.qrc

FORMS   += mainwindow.ui \

HEADERS += mainwindow.h \
           resource.h \
           resourcesmodel.h \
           settings.h \
           stunpack.h

SOURCES += main.cpp \
           mainwindow.cpp \
           resource.cpp \
           resourcesmodel.cpp \
           settings.cpp \
           stunpack.c
