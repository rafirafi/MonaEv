
#LIBTESS2_PATH

#QMAKE_CFLAGS += \
#-std=c99 -Wall -Wextra -pedantic -pedantic-errors -Wcast-align -Wcast-qual \
#-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations \
#-Wmissing-include-dirs  \
#-Wredundant-decls -Wshadow \
#-Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused -Wsign-conversion

INCLUDEPATH += $${LIBTESS2_PATH}/Include

HEADERS += \
    $${LIBTESS2_PATH}/Include/tesselator.h \
    $${LIBTESS2_PATH}/Source/bucketalloc.h \
    $${LIBTESS2_PATH}/Source/dict.h \
    $${LIBTESS2_PATH}/Source/geom.h \
    $${LIBTESS2_PATH}/Source/mesh.h \
    $${LIBTESS2_PATH}/Source/priorityq.h \
    $${LIBTESS2_PATH}/Source/sweep.h \
    $${LIBTESS2_PATH}/Source/tess.h

SOURCES += \
    $${LIBTESS2_PATH}/Source/bucketalloc.c \
    $${LIBTESS2_PATH}/Source/dict.c \
    $${LIBTESS2_PATH}/Source/geom.c \
    $${LIBTESS2_PATH}/Source/mesh.c \
    $${LIBTESS2_PATH}/Source/priorityq.c \
    $${LIBTESS2_PATH}/Source/sweep.c \
    $${LIBTESS2_PATH}/Source/tess.c


