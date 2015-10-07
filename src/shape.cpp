#include "shape.h"

#include <sstream>
#include <random>
#include <unordered_map>
#include <algorithm>

namespace {

class ShapeRNG
{
public:
    ShapeRNG() : gen_(rd_()) {}

    int randint(int max) {
        if (--max < 1) {
            return 0;
        }
        if (iengines_.count(max) == 0) {
            iengines_[max] = std::uniform_int_distribution<>(0, max);
        }
        return iengines_[max](gen_);
    }

    double randouble(double max) {
        if (max < 1.) {
            return 0.;
        }
        if (rengines_.count(max) == 0) {
            rengines_[max] = std::uniform_real_distribution<>(0, max);
        }
        return rengines_[max](gen_);
    }
private:
    std::random_device rd_;
    std::mt19937 gen_;
    std::unordered_map<int, std::uniform_int_distribution<> > iengines_;
    std::unordered_map<double, std::uniform_real_distribution<> > rengines_;
};
ShapeRNG g_rng;

#define RANDINT g_rng.randint
#define RANDDOUBLE g_rng.randouble
#define RANDUINT8() (static_cast<uint8_t>(g_rng.randint(256)))
#define RANDBOOL() (static_cast<bool>(g_rng.randint(2)))

inline int CLAMPINT(int val, int min, int max)
{
    return val < min ? min : (val > max ? max : val);
}

/*
 * init random collection
 */
std::array<float, 2> randomPoint(int width, int height)
{
    return {{ static_cast<float>(RANDINT(width + 1)), static_cast<float>(RANDINT(height + 1)) }};
}

std::vector<std::array<float, 2> > randomPolygon(int width, int height, int vertexCount)
{
    std::vector<std::array<float, 2> > polygon(vertexCount);

    for (auto &point : polygon) {
        point = randomPoint(width, height);
    }

    return polygon;
}

Shape randomShape(int width, int height, int vertexCount, int alpha)
{
    return { randomColor(alpha), randomPolygon(width, height, vertexCount) };
}

} //namespace


std::array<uint8_t, 4> randomColor(int alpha)
{
    return std::array<uint8_t, 4>({{ RANDUINT8(), RANDUINT8(), RANDUINT8(),static_cast<uint8_t>(
                                     (alpha == -1) ? CLAMPINT(RANDUINT8(), 1, 255) : alpha) }});
}

ShapeCollection randomShapeCollection(int width,
                                      int height,
                                      int polygonCount,
                                      int vertexCount,
                                      int alphaSeed)
{
    ShapeCollection shapes{ width, height, polygonCount, vertexCount,
                std::vector<Shape>(polygonCount) };

    for (Shape &shape : shapes.shapes) {
        shape = randomShape(width, height, vertexCount, alphaSeed);
    }

    return shapes;
}

/*
 * dna and svg format
 */
// TODO(rafi): whole thing is ugly, at least do EOF check
ShapeCollection fromDna(int width, int height, const std::string &dna, bool &error)
{
    if (dna.empty()) {
        error = true;
        return ShapeCollection();
    }

    error = false;

    ShapeCollection collection;
    collection.width = width;
    collection.height = height;

    std::istringstream iss(dna);
    iss >> collection.vertexCount >> collection.polygonCount;

    collection.shapes =
            std::vector<Shape>(collection.polygonCount,
                               Shape{ {{0, 0, 0, 0}},
                                      std::vector<std::array<float, 2> >(collection.vertexCount,
                                      {{0., 0.}} ) });

    for (Shape &shape : collection.shapes) {
        for (int i = 0; i < 3; i++) {
            // istringstream : uint8_t => char
            int val;
            iss >> val;
            shape.rgba[i] = val;

            error = (val < 0 || val > 255);
            if (error) {
                break;
            }
        }
        if (error) {
            break;
        }

        float a;
        iss >> a;
        shape.rgba[3] = a * 255.f + .5f;

        error = (a < 0.f || a > 1.f);
        if (error) {
            break;
        }

        for (std::array<float, 2> &pt : shape.polygon) {
            iss >> pt[0] >> pt[1];
            error = (pt[0] < 0 || pt[0] > collection.width
                    || pt[1] < 0 || pt[1] > collection.height);
            if (error) {
                break;
            }
        }
    }

    if (error) {
        return ShapeCollection();
    }

    return collection;
}

std::string getDna(const ShapeCollection &collection)
{
    std::string dna;

    dna += std::to_string(collection.vertexCount) + " ";

    dna += std::to_string(collection.polygonCount) + " ";

    for (const Shape &shape : collection.shapes) {

        for (int i = 0; i < 3; i++) {
            dna += std::to_string(shape.rgba[i]) + " ";
        }
        dna += std::to_string(shape.rgba[3] / 255.f) + " ";

        for (const std::array<float, 2> &pt : shape.polygon) {
            dna +=  std::to_string(static_cast<int>(pt[0])) + " ";
            dna +=  std::to_string(static_cast<int>(pt[1])) + " ";
        }
    }

    return dna;
}

std::string getSvg(const ShapeCollection &collection)
{
    std::string svg(
                "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
                "<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
                "xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:ev=\"http://www.w3.org/2001/xml-events\"\n"
                "version=\"1.1\" baseProfile=\"full\"\n");

    svg += "width=\"800mm\" height=\"600mm\">\n";

    for (const Shape &shape : collection.shapes) {
        svg += "<polygon points=\"";
        for (const std::array<float, 2> &pt : shape.polygon) {
            svg +=  std::to_string(static_cast<int>(pt[0])) + " ";
            svg +=  std::to_string(static_cast<int>(pt[1])) + " ";
        }
        svg.erase(--svg.end());
        //svgString += QString("\" fill=\"rgb(%1,%2,%3)\" opacity=\"%4\" />\n")
        svg += "\" fill=\"rgb(";
        for (int i = 0; i < 3; i++) {
            svg += std::to_string(shape.rgba[i]) + (i < 2 ? "," : "");
        }
        svg += ")\" opacity=\"";
        svg += std::to_string(shape.rgba[3] / 255.f);
        if (*svg.rbegin() == ' ') {
            svg.erase(--svg.end());
        }
        svg += "\" />\n";
    }
    svg += "</svg>\n";

    return svg;
}

/*
 * mutate collection
 */
Mutation setMutation(const Mutation &mutation, ShapeCollection &collection)
{
    Mutation rmutation = mutation;
    switch(mutation.type) {
    case Mutation::kOrderSwapped:
        std::swap(collection.shapes[mutation.shapeIdx], collection.shapes[mutation.data.shapeIdxB]);
        break;
    case Mutation::kColorChanged:
        rmutation.data.color = collection.shapes[mutation.shapeIdx].rgba;
        collection.shapes[mutation.shapeIdx].rgba = mutation.data.color;
        break;
    case Mutation::kPointChanged:
        rmutation.data.pt.val = collection.shapes[mutation.shapeIdx].polygon[mutation.data.pt.idx];
        collection.shapes[mutation.shapeIdx].polygon[mutation.data.pt.idx] = mutation.data.pt.val;
        break;
    default:
        ;
    }
    return rmutation;
}

Mutation getMutation(const ShapeCollection &collection)
{
    Mutation mutation{};

    double roulette = (collection.polygonCount > 1 ? RANDDOUBLE(2.2) : RANDDOUBLE(2.0));

    mutation.type = (roulette >= 2.0f ? Mutation::kOrderSwapped :
                                      (roulette < 1.0f ? Mutation::kColorChanged :
                                                       Mutation::kPointChanged));

    mutation.shapeIdx = RANDINT(collection.polygonCount);

    switch(mutation.type) {

    case Mutation::kOrderSwapped:
    {
        mutation.data.shapeIdxB = RANDINT(collection.polygonCount);
        while (mutation.shapeIdx == mutation.data.shapeIdxB) {
            mutation.data.shapeIdxB = RANDINT(collection.polygonCount);
        }
        if (mutation.shapeIdx > mutation.data.shapeIdxB) {
            std::swap(mutation.shapeIdx, mutation.data.shapeIdxB);
        }
    }
        break;

    case Mutation::kColorChanged:
    {
        mutation.data.color = collection.shapes[mutation.shapeIdx].rgba;

        bool drastic = RANDBOOL();

        int part = roulette  * 4;
        mutation.data.color[part] = (drastic ?
                                         CLAMPINT(RANDUINT8(), part == 3 ? 0 : 1, 255) :
                                         CLAMPINT(mutation.data.color[part] +
                                               (RANDBOOL() ? + 1 : -1)
                                               * RANDINT(25), part == 3 ? 0 : 1, 255));
    }
        break;

    case Mutation::kPointChanged:
    {
        mutation.data.pt.idx = RANDINT(collection.vertexCount);
        mutation.data.pt.val = collection.shapes[mutation.shapeIdx].polygon[mutation.data.pt.idx];

        bool drastic = RANDBOOL(); ;

        bool changeX = roulette < 1.5;
        std::array<float, 2> &ptRef = mutation.data.pt.val;

        if (changeX) {
            double x = ptRef[0];
            if (drastic) {
                ptRef[0] = RANDINT(collection.width + 1);
                while (x == ptRef[0]) {
                    ptRef[0] = RANDINT(collection.width + 1);
                }
            } else {
                ptRef[0] = CLAMPINT(ptRef[0] + (RANDBOOL() ? +1 : -1) *
                        static_cast<int>(RANDDOUBLE(collection.width / 10.0)), 0, collection.width);
            }
        } else {
            double y = ptRef[1];
            if (drastic) {
                ptRef[1] = RANDINT(collection.height + 1);
                while (y == ptRef[1]) {
                    ptRef[1] = RANDINT(collection.height + 1);
                }
            } else {
                ptRef[1] = CLAMPINT(ptRef[1] + (RANDBOOL() ? +1 : -1) *
                        static_cast<int>(RANDDOUBLE(collection.height / 10.0)), 0,
                        collection.height);
            }
        }
    }
        break;

    default:
        ;
    }

    return mutation;
}
