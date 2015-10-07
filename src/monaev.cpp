#include "monaev.h"

#include <stdio.h>

#include "glimageloader.h"
#include "glprograms.h"


bool MonaEv::init(const Options &options)
{
    ppainter_ = std::unique_ptr<PolygonPainter>(new PolygonPainter());
    if (!ppainter_ || ppainter_->init(options) == false) {
        printf("Failed to initialize polygon painter\n");
        return false;
    }
    return true;
}

void MonaEv::doStep()
{
    if (ppainter_) {
        ppainter_->evolveStep();
    }
}

void MonaEv::displayImage(const std::array<int, 2> &winSize)
{
    if (ppainter_) {
        displayTexture(ppainter_->getImageTexture(), ppainter_->getImageSize(), winSize);
    }
}

std::array<int, 2> MonaEv::getimageSize() const
{
    if (ppainter_) {
        return ppainter_->getImageSize();
    }
    return {{ 0, 0 }};
}

void MonaEv::displayShapes(const std::array<int, 2> &winSize)
{
    if (ppainter_) {
        displayTexture(ppainter_->getShapesTexture(), ppainter_->getImageSize(), winSize);
    }
}

void MonaEv::printDna() const
{
    if (ppainter_) {
        printf("%s\n", ppainter_->getDna().c_str());
    }
}

void MonaEv::printSvg() const
{
    if (ppainter_) {
        printf("%s\n", ppainter_->getSvg().c_str());
    }
}

void MonaEv::printInfo() const
{
    if (!ppainter_) {
        return;
    }
    PolygonPainter::Info info = ppainter_->getInfo();
    static std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    if (info.mutationCnt == 0) {
        printf("%15s %10s %10s %10s %5s %6s %6s\n",
                "score", "mutation", "improv.", "neutral", "child.", "fitness", "change/s");
        lastTime = info.lastTime_;
    }
    if (lastTime != info.lastTime_) {
        lastTime = info.lastTime_;
        printf("%15" PRIu64 " %10d %10d %10d %6d %.3f  %.2f\n", info.score, info.mutationCnt,
                info.improvementCnt, info.neutralCnt, ppainter_->getStepSize(),
                info.getFitness(), info.changesPerSecond);
    }
}

void MonaEv::displayTexture(GLuint texture, const std::array<int, 2> &textureSize, const std::array<int, 2> &winSize)
{
    glViewport(0, 0, winSize[0], winSize[1]);

    if (texDrawerProg_.programId() == 0) {
        texDrawerProg_.init(texDrawerVert, texDrawerFrag, textureSize[0], textureSize[1]);
        if (texDrawerProg_.programId() == 0) {
            printf("Error while trying to display image... aborting\n");
            ppainter_.reset();
        }
        glUseProgram(texDrawerProg_.programId());
        texDrawerProg_.setOrtho(textureSize[0], textureSize[1]);
        texDrawerProg_.setTextureSize(textureSize[0], textureSize[1]);
        texDrawerProg_.setTextureUnit(0);
        glUseProgram(0);

        quadDrawer_.init(textureSize[0], textureSize[1]);
        quadDrawer_.setSize(textureSize[0], textureSize[1]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(texDrawerProg_.programId());
    quadDrawer_.draw();
    glUseProgram(0);


    glViewport(0, 0,  textureSize[0], textureSize[1]);
}
