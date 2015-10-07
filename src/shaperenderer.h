#ifndef SHAPERENDERER_H
#define SHAPERENDERER_H

#include <vector>

#include <GL/glew.h>

#include "tesspoly2d.h"
#include "glutils.h"
#include "glquaddrawer.h"

#include "shape.h"
#include "shapeslice.h"

class ShapeRenderer
{
public:
    ShapeRenderer()=default;
    ~ShapeRenderer();

    bool init(const ShapeCollection &collection, int sliceNbTarget = 0);

    GLuint render();

    void update(const Mutation &mutation, const ShapeCollection &collection);

private:
    int shapeSlicesSize_ = 0;
    std::vector<ShapeSlice> shapeSlices_;

    TessPoly2D tObj_;

    int fboAttachmentCount_ = 0;
    std::vector<GLuint> fbos_;

    std::vector<GLuint> texPlanes_; // for rendering and storing slices rendered
    std::vector<bool> planesValid_;
    std::vector<GLuint> texComposed_; // for storing intermediate composition result

    Program colorDrawerProg_;
    Program texDrawerProg_;
    QuadDrawer quadDrawer_;

    std::pair<int, int> getSliceFboTextureIdx(int sliceIdx) const;

    int  shapesPerSliceMax_ = 0;
    int  sliceNbTarget_ = 1;
};

#endif // SHAPERENDERER_H
