TEMPLATE = app

CONFIG += warn_on qt

QT += opengl widgets

TARGET = stressed

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

win32 {
  CONFIG(release, debug|release) {
    PRE_TARGETDEPS += ../animation/release/libanimation.a \
                      ../bitmap/release/libbitmap.a \
                      ../raw/release/libraw.a \
                      ../shape/release/libshape.a \
                      ../speed/release/libspeed.a \
                      ../text/release/libtext.a
  }
  else {
    PRE_TARGETDEPS += ../animation/debug/libanimation.a \
                      ../bitmap/debug/libbitmap.a \
                      ../raw/debug/libraw.a \
                      ../shape/debug/libshape.a \
                      ../speed/debug/libspeed.a \
                      ../text/debug/libtext.a
  }
  LIBS += $$PRE_TARGETDEPS -lopengl32 -lglu32

  QMAKE_LFLAGS += -static-libgcc -static-libstdc++

  RC_FILE = ../../resources/resources-win32.rc
}
else {
  PRE_TARGETDEPS += ../animation/libanimation.a \
                    ../bitmap/libbitmap.a \
                    ../raw/libraw.a \
                    ../shape/libshape.a \
                    ../speed/libspeed.a \
                    ../text/libtext.a

  LIBS += $$PRE_TARGETDEPS -lGLU
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
