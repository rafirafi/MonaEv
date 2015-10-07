#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <string>

#include <GL/glew.h>

#include "shape.h"
#include "shaperenderer.h"

class ShapeManager
{
public:
    ShapeManager()=default;
    ~ShapeManager()=default;

    bool init(int width, int height, int polygonCount, int vertexCount, int sliceNb = 0);

    bool init(int width, int height, const std::string &dna, int sliceNb = 0);

    Mutation mutate();

    void revMutate();

    void applyMutation(const Mutation &mutation);

    GLuint render();

    std::string getDna() const;

    std::string getSvg() const;

    std::array<int, 2> getPolygonVertexCount() const;

    Mutation getRevMutation() const;

private:
    ShapeRenderer renderer_;
    ShapeCollection collection_;

    Mutation mutation_;
    Mutation revMutation_;

    bool initRenderer(int sliceNb);
    int calibrate();
};

#endif // SHAPEMANAGER_H
