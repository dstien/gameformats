TEMPLATE = app

CONFIG += warn_on qt

QT += opengl

TARGET = stressed

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

win32 {
  CONFIG(release, debug|release) {
    PRE_TARGETDEPS += ../bitmap/release/libbitmap.a \
                      ../shape/release/libshape.a \
                      ../text/release/libtext.a
  }
  else {
    PRE_TARGETDEPS += ../bitmap/debug/libbitmap.a \
                      ../shape/debug/libshape.a \
                      ../text/debug/libtext.a
  }
  LIBS += $$PRE_TARGETDEPS -lopengl32 -lglu32

  RC_FILE = ../../resources/resources-win32.rc
}
else {
  PRE_TARGETDEPS += ../bitmap/libbitmap.a \
                    ../shape/libshape.a \
                    ../text/libtext.a

  LIBS += $$PRE_TARGETDEPS
}

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
