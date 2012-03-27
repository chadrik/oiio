#-------------------------------------------------
#
# Project created by QtCreator 2012-03-23T10:18:40
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = madFramebuffer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    preferencesdialog.cpp \
    imagebuffer.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    preferencesdialog.h \
    ivimage.h \
    imagebuffer.h

FORMS    += mainwindow.ui \
    preferencesdialog.ui


# LINUX
unix:!macx:!symbian: LIBS += -lGLEW

unix:!macx:!symbian: LIBS += -L$$PWD/../../oiio/dist/linux64/lib/ -lOpenImageIO

INCLUDEPATH += $$PWD/../../oiio/dist/linux64/include
DEPENDPATH += $$PWD/../../oiio/dist/linux64/include

unix:!macx:!symbian: LIBS += -L$$PWD/../../../../madcrew/applications/SDK/ocio/lib/ -lOpenColorIO

unix:!macx:!symbian: INCLUDEPATH += $$PWD/../../../../madcrew/applications/SDK/ocio/include
unix:!macx:!symbian: DEPENDPATH += $$PWD/../../../../madcrew/applications/SDK/ocio/include

# OSX
macx: INCLUDEPATH += /opt/local/include
macx: LIBS += -L$$PWD/../oiio/dist/macosx/lib/ -lOpenImageIO

macx: INCLUDEPATH += $$PWD/../oiio/dist/macosx/include
macx: DEPENDPATH += $$PWD/../oiio/dist/macosx/include

macx: LIBS += -lOpenColorIO

macx: LIBS += -L/opt/local/lib -lGLEW

