#include "polygonpainter.h"

#include <algorithm>
#include <chrono>

#include "glimageloader.h"

PolygonPainter::~PolygonPainter()
{
    glDeleteTextures(1, &imageTexture_);
}

bool PolygonPainter::init(const Options &options)
{
    imagePath_ = options.painter.imagePath;
    imageTexture_ = loadImage(imagePath_, imageSize_[0], imageSize_[1]);
    if (imageTexture_ == 0) {
        fprintf(stderr, "%s loadImage failed %s\n", __PRETTY_FUNCTION__ , imagePath_.c_str());
        return false;
    }
    glViewport(0, 0, imageSize_[0], imageSize_[1]);

    if (options.manager.dna.empty() == false) {
        if (manager_.init(imageSize_[0],
                          imageSize_[1],
                          options.manager.dna,
                          options.manager.sliceCount) == false) {
            return false;
        }
    } else {
        if (options.manager.polygonCount < 1
                || options.manager.vertexCount < 3
                || manager_.init(imageSize_[0],
                                 imageSize_[1],
                                 options.manager.polygonCount,
                                 options.manager.vertexCount,
                                 options.manager.sliceCount) == false) {
            return false;
        }
    }
    dropNeutral_ = options.painter.dropNeutral;
    stepSize_ = options.painter.stepSize;
    stepSizeMax_ = options.painter.stepSize;

    return init(options.evaluator.useSSd, options.evaluator.force16Bit);
}

bool PolygonPainter::init(bool useSsd, bool force16Bit)
{
    fprintf(stderr, "image path %s\n", imagePath_.c_str());
    fprintf(stderr, "image size %d * %d\n", imageSize_[0], imageSize_[1]);

    auto polyVertCnt = manager_.getPolygonVertexCount();
    fprintf(stderr, "polygon count %d vertex count %d\n", polyVertCnt[0], polyVertCnt[1]);

    if (sumComputer_.init(imageSize_, force16Bit) == false) {
        return false;
    }
    sumComputer_.setSdMethod(useSsd ? SumComputer::Method::SSD : SumComputer::Method::SAD);
    fprintf(stderr, "Using sum of %s differences\n", useSsd ? "squared" : "absolute");

    setStepSize(stepSize_, stepSizeMax_);
    fprintf(stderr, "Max descendant nb %d\n", sumComputer_.sdMaxStorable());

    uint64_t imageSum = sumComputer_.sum(imageTexture_);
    fprintf(stderr, "Image sum      %" PRIu64 "\n", imageSum);

    info_.worstScore = sumComputer_.maxSd(imageTexture_);

    fprintf(stderr, "Shapes max sd  %" PRIu64 "\n", info_.worstScore);

    GLuint shapesTexture = manager_.render();
    info_.score = sumComputer_.sd(imageTexture_, shapesTexture);

    fprintf(stderr, "Shapes sd      %" PRIu64 "\n", info_.score);

    return true;
}

void PolygonPainter::evolveStep()
{
    if (info_.mutationCnt == 0) {
        info_.lastTime_ = std::chrono::high_resolution_clock::now();
    } else {
        auto now = std::chrono::high_resolution_clock::now();
        double usElapsed = static_cast<double>(std::chrono::duration_cast
                                               <std::chrono::microseconds>
                                               (now - info_.lastTime_).count());
        if (usElapsed > 1000000) {
            double changeDelta = info_.mutationCnt - info_.lastMutationCnt_;
            info_.changesPerSecond = changeDelta / (usElapsed / 1000000.);
            info_.lastTime_ = now;
            info_.lastMutationCnt_ = info_.mutationCnt;
        }
    }

    info_.mutationCnt += stepSize_;

    std::vector<Mutation> mutations(stepSize_);

    sumComputer_.sdStoredBegin();

    for (int i = 0; i < stepSize_; i++) {

        mutations[i] = manager_.mutate();

        GLuint shapesTexture = manager_.render();
        sumComputer_.sdStoredNext(imageTexture_, shapesTexture);

        manager_.revMutate();

        glFlush(); // there is work to do, now.
    }

    std::vector<uint64_t> sds = sumComputer_.sdStoredEnd();

    int bestIdx = std::distance(sds.begin(), std::min_element(sds.begin(), sds.end()));

    if (sds[bestIdx] < info_.score) {
        info_.improvementCnt++;
    } else {
        stepSize_ = std::min(stepSize_ + 1, stepSizeMax_);
        if (!dropNeutral_ && sds[bestIdx] == info_.score) {
            info_.neutralCnt++;
        } else {
            return;
        }
    }

    info_.score = sds[bestIdx];

    manager_.applyMutation(mutations[bestIdx]);
}

GLuint PolygonPainter::getShapesTexture()
{
    return manager_.render();
}

GLuint PolygonPainter::getImageTexture() const
{
    return imageTexture_;
}

PolygonPainter::Info PolygonPainter::getInfo() const
{
    return info_;
}

std::string PolygonPainter::getDna() const
{
    return manager_.getDna();
}

std::string PolygonPainter::getSvg() const
{
    return manager_.getSvg();
}

std::string PolygonPainter::getImagePath()
{
    return imagePath_;
}

std::array<int, 2> PolygonPainter::getImageSize() const
{
    return imageSize_;
}

void PolygonPainter::dropNeutral(bool drop)
{
    dropNeutral_ = drop;
}

int PolygonPainter::getStepSize() const
{
    return stepSize_;
}

void PolygonPainter::setStepSize(int stepSize, int stepSizeMax)
{
    if (stepSizeMax == 0) {
        stepSizeMax = stepSize;
    }
    stepSizeMax_ = std::min(stepSizeMax, sumComputer_.sdMaxStorable());
    stepSize_  = std::min(stepSize, stepSizeMax_);
}
