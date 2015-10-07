#ifndef POLYGONPAINTER_H
#define POLYGONPAINTER_H

#include <string>
#include <array>
#include <chrono>

#include "shapemanager.h"
#include "sumcomputer.h"
#include "options.h"

class PolygonPainter
{
public:

    struct Info {
        int mutationCnt = 0;
        int improvementCnt = 0;
        int neutralCnt = 0;

        uint64_t worstScore = 0;
        uint64_t score = 0;
        float getFitness() const { return 100 * (double)(worstScore - score) / (double)worstScore; }

        float changesPerSecond = 0.f;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastTime_;
        int lastMutationCnt_ = 0;
    };

    PolygonPainter()=default;
    ~PolygonPainter();

    /*
     * NOTE: glViewport set in init(), the caller is responsible to restore it if it changes.
     * Necessary for fbo consumers (renderer and sum computer)
     */
    bool init(const Options &options);

    void evolveStep();

    Info getInfo() const;

    GLuint getShapesTexture();

    GLuint getImageTexture() const;

    std::string getDna() const;

    std::string getSvg() const;

    std::string getImagePath();

    std::array<int, 2> getImageSize() const;

    void dropNeutral(bool drop);

    int getStepSize() const;

    void setStepSize(int stepSize, int stepSizeMax = 0);

private:
    std::string imagePath_;
    std::array<int, 2> imageSize_;
    GLuint imageTexture_ = 0;

    ShapeManager manager_;
    SumComputer sumComputer_;

    int stepSize_ = 25;
    int stepSizeMax_ = 25;

    bool dropNeutral_ = false;

    Info info_;

    bool init(bool useSsd, bool force16Bit);
};

#endif // POLYGONPAINTER_H
