TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = ../

QMAKE_CXXFLAGS += -std=c++11

# needed for portable printf format macro (PRIu64 ...) in a c++ conttext
DEFINES += __STDC_FORMAT_MACROS

CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS_RELEASE += -O3
}

INCLUDEPATH += utils/

CONFIG(unix) {

    TARGET = monaev

    CONFIG += link_pkgconfig

    PKGCONFIG += libpng

    PKGCONFIG += glew

    PKGCONFIG += libglfw
}

CONFIG(win32) {
    WIN32DIR = MinGW32-4.9.1

    TARGET = ./$${WIN32DIR}/monaev

    INCLUDEPATH += ../$${WIN32DIR}/glew/include/
    LIBS += -L../$${WIN32DIR}/glew/lib/ -lglew32

    INCLUDEPATH += ../$${WIN32DIR}/png/include/libpng12/
    LIBS += -L../$${WIN32DIR}/png/lib/ -lpng12 -lz

    DEFINES += GLFW_DLL
    INCLUDEPATH += ../$${WIN32DIR}/glfw2/include/
    LIBS += -L../$${WIN32DIR}/glfw2/lib-mingw/ -lglfw

    LIBS += -lopengl32
}

SOURCES += \
    main.cpp \
    shape.cpp \
    glprograms.cpp \
    shapemanager.cpp \
    polygonpainter.cpp \
    monaev.cpp \
    sumcomputer.cpp \
    shaperenderer.cpp \
    shapeslice.cpp \
    utils/glimageloader.cpp \
    utils/glmatrix.cpp \
    utils/glquaddrawer.cpp \
    utils/glutils.cpp \
    utils/tesspoly2d.cpp

HEADERS += \
    shape.h \
    glprograms.h \
    shapemanager.h \
    polygonpainter.h \
    monaev.h \
    sumcomputer.h \
    shaperenderer.h \
    shapeslice.h \
    options.h \
    utils/glimageloader.h \
    utils/glmatrix.h \
    utils/glquaddrawer.h \
    utils/glutils.h \
    utils/tesspoly2d.h

#integrated lib
LIBTESS2_PATH=./libtess2
include($${LIBTESS2_PATH}/libtess2.pri)
EZOPTIONPARSER_PATH=./ezOptionParser
include($${EZOPTIONPARSER_PATH}/ezOptionParser.pri)
