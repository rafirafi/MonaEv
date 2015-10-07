#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <array>
#include <string>

#include <stdint.h>

struct Shape
{
    std::array<uint8_t, 4> rgba;
    std::vector<std::array<float, 2> > polygon;
};

struct ShapeCollection
{
    int width;
    int height;
    int polygonCount;
    int vertexCount;
    std::vector<Shape> shapes;
};

struct Mutation {
    enum {
        kOrderSwapped,
        kColorChanged,
        kPointChanged,
        kNone
    };

    int type = kNone;
    int shapeIdx = 0;

    union {
        // kOrderSwapped
        int shapeIdxB;
        // kColorChanged
        std::array<uint8_t, 4> color;
        // kPointChanged
        struct {
           int idx;
           std::array<float, 2> val;
        } pt;
    } data;
};

// alphaSeed : -1 for random, 0-255 for a fixed seed value
ShapeCollection randomShapeCollection(int width,
                                        int height,
                                        int polygonCount,
                                        int vertexCount,
                                        int alphaSeed);

std::array<uint8_t, 4> randomColor(int alpha);

ShapeCollection fromDna(int width, int height, const std::string &dna, bool &error);

std::string getDna(const ShapeCollection &collection);

std::string getSvg(const ShapeCollection &collection);

Mutation getMutation(const ShapeCollection &collection);

Mutation setMutation(const Mutation &mutation, ShapeCollection &collection);

#endif // SHAPE_H
