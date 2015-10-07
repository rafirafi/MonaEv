
#include <stdio.h>
#include <assert.h>

#include <vector>
#include <array>

#include "tesselator.h"

int main()
{
    // create tesselator
    TESStesselator* tess = nullptr;

    tess = tessNewTess(0);
    assert(tess);



    // input contour
    std::vector<std::array<float, 2> > polyline =
    {
        {159, 31},
        {80, 52},
        {7, 69},
        {128, 166},
        {177, 63},
        {95, 69},
        {154, 115}
    };

    int size = 2;
    void *pointer = &polyline[0][0];
    int stride = 2 * sizeof(float);
    int count = 2 * polyline.size();

    tessAddContour(tess, size, pointer, stride, count);



    // do tesselation
    int polySize = 3; // ouput only triangle, if > 3 => GL_TRIANGLE_FAN
    int vertexSize = 3; // from 2 pts vrtex to 3 pts vertex
    TESSreal* normal = nullptr;

    int ret = tessTesselate(tess,
                            TESS_WINDING_NONZERO,
                            TESS_POLYGONS,
                            polySize,
                            vertexSize,
                            normal
                            );
    assert(ret == 1);



    // retrieve triangles
    const int nelems = tessGetElementCount(tess);
    const TESSindex* elems = tessGetElements(tess);
    const float* verts = tessGetVertices(tess);

    fprintf(stderr, "nelems %d\n", nelems);

    for (int i = 0; i < nelems; i++) {
        const TESSindex* poly = &elems[i * polySize];

        fprintf(stderr, "elems %d\n", i);

        for (int j = 0; j < polySize; j++) {
            if (poly[j] == TESS_UNDEF) {
                fprintf(stderr, "poly[j] == TESS_UNDEF\n");
                break;
            }
            fprintf(stderr, "%f %f %f\n", verts[poly[j] * vertexSize],
                    verts[poly[j] * vertexSize + 1],
                    verts[poly[j] * vertexSize + 2]);
        }

    }

    // delete tesselator
    tessDeleteTess(tess);



    return 0;
}

/*

  ideas => tessTesselate : polySize, vertexSize
  name not enough explicit, not clear if it defines output when first read

  test if tessTesselate forget contour once outputed

 */
