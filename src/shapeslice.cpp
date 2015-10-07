#include "shapeslice.h"

#include <algorithm>

#include "tesspoly2d.h"
#include "glutils.h"

ShapeGl tesselate(const Shape &shape, TessPoly2D &tObj)
{
    ShapeGl shapeGl;
    tObj.tesselate(shape.polygon, &shapeGl.positions, nullptr);
    shapeGl.rgba = shape.rgba;
    return shapeGl;
}

void ShapeSlice::free()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboPos);
    glDeleteBuffers(1, &vboCol);
}

void ShapeSlice::init(TessPoly2D &tObj, const ShapeCollection &collection, int first, int last)
{
    int shapeCnt = last - first + 1;
    shapeRange = {{ first, last }};
    shapePosOffSize.resize(shapeCnt, {{ 0, 0}});
    shapeGls.resize(shapeCnt);
    positions.clear();
    colors.clear();

    for (int i = 0; i < shapeCnt; i++) {
        shapeGls[i] = tesselate(collection.shapes[first + i], tObj);

        int posOff = (i == 0 ? 0 : shapePosOffSize[i - 1][0] + shapePosOffSize[i - 1][1]);
        int posSize = shapeGls[i].positions.size();
        shapePosOffSize[i] = {{ posOff, posSize }};

        positions.insert(positions.end(), shapeGls[i].positions.begin(), shapeGls[i].positions.end());
        colors.insert(colors.end(), shapeGls[i].positions.size(), shapeGls[i].rgba);
    }

    initGlObjects();
}

void ShapeSlice::initGlObjects()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboPos);
    glGenBuffers(1, &vboCol);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glEnableVertexAttribArray(ShName::kACOO2D);
    glVertexAttribPointer(ShName::kACOO2D, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboCol);
    glEnableVertexAttribArray(ShName::kACOLOR);
    glVertexAttribPointer(ShName::kACOLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ShapeSlice::render()
{
    updateGlObjects();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, positions.size());
    glBindVertexArray(0);
}

void ShapeSlice::updateGlObjects()
{
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);

    glBufferData(GL_ARRAY_BUFFER,
                 positions.size() * sizeof(std::array<float, 2>),
                 NULL,
                 GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    positions.size() * sizeof(std::array<float, 2>),
                    positions.data());

    glBindBuffer(GL_ARRAY_BUFFER, vboCol);

    glBufferData(GL_ARRAY_BUFFER,
                 colors.size() * sizeof(std::array<uint8_t, 4>),
                 NULL,
                 GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    colors.size() * sizeof(std::array<uint8_t, 4>),
                    colors.data());
}

void ShapeSlice::update(const ShapeCollection &collection, int shapeIdx, TessPoly2D &tObj)
{
    ShapeGl shapeGl = tesselate(collection.shapes[shapeIdx], tObj);

    int idxA = shapeIdx - shapeRange[0];
    int shift = shapeGl.positions.size() - shapePosOffSize[idxA][1];

    // adjust size for new data
    if (shift != 0) {
        if (shift < 0) {
            positions.erase(positions.begin() + shapePosOffSize[idxA][0],
                    positions.begin() + (shapePosOffSize[idxA][0] - shift));

            colors.erase(colors.begin() + shapePosOffSize[idxA][0],
                    colors.begin() + (shapePosOffSize[idxA][0] - shift));

        } else if (shift > 0) {
            positions.insert(positions.begin() + (shapePosOffSize[idxA][0]),
                    shift, std::array<float, 2>());

            colors.insert(colors.begin() + shapePosOffSize[idxA][0],
                    shift, shapeGl.rgba);
        }

        // adjust size and propagate offset changes
        shapePosOffSize[idxA][1] += shift;
        for (int i = idxA + 1; i < shapeRange[1] - shapeRange[0] + 1; i++) {
            shapePosOffSize[i][0] += shift;
        }
    }

    // insert new data
    std::copy(shapeGl.positions.begin(),
              shapeGl.positions.end(),
              positions.begin() + shapePosOffSize[idxA][0]);

    std::for_each(colors.begin() + shapePosOffSize[idxA][0],
            colors.begin() + (shapePosOffSize[idxA][0] + shapePosOffSize[idxA][1]),
            [&](std::array<uint8_t, 4> &color) {
        color = shapeGl.rgba;
    });

}
