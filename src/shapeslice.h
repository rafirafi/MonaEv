#ifndef SHAPESLICE_H
#define SHAPESLICE_H

#include <stdint.h>

#include <array>
#include <vector>

#include <GL/glew.h>

#include "shape.h"

class TessPoly2D;

struct ShapeGl
{
    std::array<uint8_t, 4> rgba;
    std::vector<std::array<float, 2> > positions;
};

ShapeGl tesselate(const Shape &shape, TessPoly2D &tObj);

struct ShapeSlice
{
    ShapeSlice()=default;

    void free();

    void init(TessPoly2D &tObj, const ShapeCollection &collection, int first, int last);

    void render();

    void update(const ShapeCollection &collection, int shapeIdx, TessPoly2D &tObj);

private:
    std::array<int, 2> shapeRange; // [,]
    std::vector<ShapeGl> shapeGls;

    std::vector<std::array<int, 2> > shapePosOffSize;
    std::vector<std::array<float, 2> > positions;
    std::vector<std::array<uint8_t, 4> > colors;
    std::vector<unsigned int> indices;

    GLuint vao, vboPos, vboCol;

    void initGlObjects();
    void updateGlObjects();
};

#endif // SHAPESLICE_H
