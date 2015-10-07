#ifndef MONAEV_H
#define MONAEV_H

#include <GL/glew.h>

#include <memory>

#include "glquaddrawer.h"
#include "glutils.h"
#include "options.h"
#include "polygonpainter.h"

class MonaEv
{
public:
    MonaEv()=default;
    ~MonaEv()=default;

    bool init(const Options &options);

    void doStep();

    std::array<int, 2> getimageSize() const;

    void displayImage(const std::array<int, 2> &winSize);

    void displayShapes(const std::array<int, 2> &winSize);

    void printDna() const;

    void printSvg() const;

    void printInfo() const;

private:
    std::unique_ptr<PolygonPainter> ppainter_;
    QuadDrawer quadDrawer_;
    Program texDrawerProg_;

    void displayTexture(GLuint texture, const std::array<int, 2> &textureSize,
                        const std::array<int, 2> &winSize);
};

#endif // MONAEV_H
