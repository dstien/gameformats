TEMPLATE = app

CONFIG += warn_on qt

QT += opengl widgets

TARGET = stressed

DEPENDPATH  += ..
INCLUDEPATH += $$DEPENDPATH

win32 {
  CONFIG(release, debug|release) {
    PRE_TARGETDEPS += ../animation/release/animation.lib \
                      ../bitmap/release/bitmap.lib \
                      ../raw/release/raw.lib \
                      ../shape/release/shape.lib \
                      ../speed/release/speed.lib \
                      ../text/release/text.lib
  }
  else {
    PRE_TARGETDEPS += ../animation/debug/animation.lib \
                      ../bitmap/debug/bitmap.lib \
                      ../raw/debug/raw.lib \
                      ../shape/debug/shape.lib \
                      ../speed/debug/speed.lib \
                      ../text/debug/text.lib
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
