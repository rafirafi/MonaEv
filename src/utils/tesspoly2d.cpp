#include "tesspoly2d.h"

#include <assert.h>

#include <algorithm>

#include "tesselator.h"

static void initVertexIndices(TESStesselator *tObj,
                              std::vector<std::array<float, 2> > &vertexPosition,
                              std::vector<unsigned int> *index)
{
    const int nelems = tessGetElementCount(tObj);
    const TESSindex* elems = tessGetElements(tObj);
    const float* verts = tessGetVertices(tObj);
    const int nverts = tessGetVertexCount(tObj);

    vertexPosition.reserve(nverts);

    int polySize = 3;

    if (index) {
        index->reserve(nelems * polySize);
        int vertexSize = 2;
        for (int i = 0; i < nverts * vertexSize; i += vertexSize) {
            vertexPosition.push_back({{verts[i], verts[i + 1]}});
        }
        for (int i = 0; i < nelems * polySize; i++) {
            index->push_back(elems[i]);
        }
    } else {
        vertexPosition.reserve(nelems * polySize);
        for (int i = 0; i < nelems * polySize; i++) {
            int idx = elems[i];
            vertexPosition.push_back({{verts[2 * idx], verts[2 * idx + 1]}});
        }
    }

    //fprintf(stderr, "%s tri nb %d\n", __func__, nelems * polySize);
}

static void tesselateShapeTris(TESStesselator *tObj,
                               const std::vector<std::array<float, 2> > &polygon)
{
    int size = 2;
    const void* pointer = &polygon[0][0];
    int stride = 2 * sizeof(float);
    int count = polygon.size();

    tessAddContour(tObj, size, pointer, stride, count);

    int polySize = 3;  // 3 : GL_TRIANGLES only, if > 3 : libtess2 output GL_TRIANGLE_FAN
    int vertexSize = 2;
    const TESSreal* normal = nullptr;

    int ret = tessTesselate(tObj,
                            TESS_WINDING_NONZERO,
                            TESS_POLYGONS,
                            polySize,
                            vertexSize,
                            normal
                            );
    if (ret != 1) {
        fprintf(stderr, "%s failed\n", __func__);
    }
}


void TessPoly2D::tesselate(const std::vector<std::array<float, 2> > &polygon,
                           std::vector<std::array<float, 2> > *vertices,
                           std::vector<unsigned int> *indices)
{
    if (indices) {
        indices->clear();
    }
    vertices->clear();
    if (polygon.size() == 3) {
        if (indices) {
            *indices = std::vector<unsigned int>{0, 1, 2};
        }
        *vertices = std::vector<std::array<float, 2> > {
            {{static_cast<float>(polygon[0][0]), static_cast<float>(polygon[0][1])}},
            {{static_cast<float>(polygon[1][0]), static_cast<float>(polygon[1][1])}},
            {{static_cast<float>(polygon[2][0]), static_cast<float>(polygon[2][1])}} };
    } else {
        tesselateShapeTris(tObj_, polygon);
        initVertexIndices(tObj_, *vertices, indices);
    }
}


TessPoly2D::TessPoly2D()
{
    tObj_ = tessNewTess(nullptr);
}

TessPoly2D::~TessPoly2D()
{
    if (tObj_) {
        tessDeleteTess(tObj_);
    }
}
