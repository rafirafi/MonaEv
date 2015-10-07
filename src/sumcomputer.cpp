#include "sumcomputer.h"

#include "glprograms.h"

// last integer which can be represented without gap
// float : 2^24 = 16777216 => only 3 reduction possible with ssd before possible overflow
// half float : 2048 => packed : 2048^4 + 2048^3 + 2048^2 + 2048 = 17600780175360
#define MAX_FLOAT32_INT 16777216

namespace
{
// return x, y, x + width, y + height
std::array<int, 4> getStoredCoo(const std::array<int, 2> &size,
                                const std::array<int, 2> &resSize,
                                int storedIdx)
{
    int width = size[0] / resSize[0];
    int x = (storedIdx %  width) * resSize[0];
    int y = (storedIdx / width) * resSize[1];
    return {{ x, y, x + resSize[0], y + resSize[1] }};
}

// width, height
std::array<int, 2> getStoredBoundedSize(const std::array<int, 2> &size,
                                        const std::array<int, 2> &resSize,
                                        int storedCount)
{
    storedCount--;
    int width = size[0] / resSize[0];
    int y = (storedCount / width) * resSize[1];
    return {{ y ? size[0] : (storedCount %  width + 1) * resSize[0], y + resSize[1]}};
}

std::array<int, 2> getMinDimCutOff(int width,
                                   int height,
                                   uint64_t maxPixelVal,
                                   uint64_t maxStorableVal)
{
    while (width > 1 && height > 1) {
        maxPixelVal <<= 2;
        if (maxPixelVal > maxStorableVal) {
            break;
        }
        width = (width >> 1) + (width & 1);
        height = (height >> 1) + (height & 1);
    }
    return {{ width, height }};
}

static inline int getMaxStorable_(const std::array<int, 2> &size, const std::array<int, 2> &resSize)
{
    return (size[0] / resSize[0]) * (size[1] / resSize[1]);
}

static inline std::array<int, 2> reduceSize(const std::array<int, 2> &size)
{
    return {{ (size[0] >> 1) + (size[0] & 1), (size[1] >> 1) + (size[1] & 1) }};
}

}  // namespace

SumComputer::~SumComputer()
{
    glDeleteTextures(textures_.size(), textures_.data());
    glDeleteFramebuffers(1, &fbo_);
}

bool SumComputer::init(const std::array<int, 2> &size, bool forceRGBA16FPacked)
{
    size_ = size;

    // don't bother with float texture with 2.1 ; broken or <= half texture perf
    int major = 0, minor = 0;
    const char *verStr = (const char *)glGetString(GL_VERSION);
    if ((!verStr) || (sscanf(verStr,"%2d.%2d", &major, &minor) != 2))
    {
        major = minor = 0;
        fprintf(stderr, "%s Could'nt get GL version\n", __func__);
        return false;
    }

    useRGBA16FPacked_ = (forceRGBA16FPacked || major < 3);
    fprintf(stderr, "using texture float %d bits\n", useRGBA16FPacked_ ? 16 : 32);

    fbo_ = prepareFbo(size_[0], size_[1],
            useRGBA16FPacked_ ? GL_RGBA16F : GL_R32F, GL_RGBA, GL_FLOAT,
            textures_.data(), textures_.size());
    if (fbo_ == 0) {
        return false;
    }

    quadDrawer_.init(size_[0], size_[1]);

    // check them
    absDiffProg_.init(texDiffVert, texAbsDiffFrag, size_[0], size_[1]);
    if (absDiffProg_.programId() == 0) {
        return false;
    }

    squaredDiffProg_.init(texDiffVert,
                          useRGBA16FPacked_ ? tex16SquaredDiffFrag : texSquaredDiffFrag,
                          size_[0], size_[1]);
    if (squaredDiffProg_.programId() == 0) {
        return false;
    }

    sumProg_.init(texSumComputerVert,
                  useRGBA16FPacked_ ? tex16SumComputerFrag : tex32SumComputerFrag,
                  size_[0], size_[1]);
    if (sumProg_.programId() == 0) {
        return false;
    }

    texNormalizeProg_.init(texDrawerVert, texNormalizeDrawerFrag, size_[0], size_[1]);
    if (texNormalizeProg_.programId() == 0) {
        return false;
    }

    storeSumProg_.init(texDrawerVert, texStoreFrag, size_[0], size_[1]);
    if (storeSumProg_.programId() == 0) {
        return false;
    }

    maxAbsDiffProg_.init(texDrawerVert, texMaxAbsDiffFrag, size_[0], size_[1]);
    if (maxAbsDiffProg_.programId() == 0) {
        return false;
    }

    maxSquaredDiffProg_.init(texDrawerVert,
                             useRGBA16FPacked_ ? tex16MaxSquaredDiffFrag : texMaxSquaredDiffFrag,
                             size_[0], size_[1]);
    if (maxSquaredDiffProg_.programId() == 0) {
        return false;
    }

    if (!useRGBA16FPacked_) {
        sadDimCutOff_ = getMinDimCutOff(size_[0], size_[1], 255 * 4, MAX_FLOAT32_INT);
        ssdDimCutOff_ = getMinDimCutOff(size_[0], size_[1], 255 * 255 * 4, MAX_FLOAT32_INT);
    }

    sadMaxStorable_ = getMaxStorable_(size_, sadDimCutOff_);
    ssdMaxStorable_ = getMaxStorable_(size_, ssdDimCutOff_);

    return true;
}

// what's not compute can be grouped together
void SumComputer::applyProgram(Program &prog, GLuint textureA, int drawBufIdx, GLuint textureB)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    GLenum bufId = GL_COLOR_ATTACHMENT0 + drawBufIdx;
    glDrawBuffers(1, &bufId);

    glBindTexture(GL_TEXTURE_2D, textureA);

    if (textureB != 0) {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, textureB);
    }

    glUseProgram(prog.programId());

    prog.setTextureUnit(0);

    if (textureB != 0) {
        prog.setTextureUnit(1, 1);
    }

    prog.setTextureSize(size_[0], size_[1]);

    quadDrawer_.setSize(size_[0], size_[1]);
    quadDrawer_.draw();

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (textureB != 0) {
        glActiveTexture(GL_TEXTURE0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint64_t SumComputer::applyProgramAndSum(std::array<int, 2> &cutOff,
                                         Program &prog,
                                         GLuint textureA,
                                         GLuint textureB)
{
    int drawTexId = kTexSumFirst;
    applyProgram(prog, textureA, drawTexId, textureB);
    drawTexId = reduceSum(cutOff);
    return readSum(cutOff, drawTexId);
}

int SumComputer::reduceSum(const std::array<int, 2> &cutOff)
{
    int drawTexId = kTexSumFirst;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
    glUseProgram(sumProg_.programId());

    sumProg_.setTextureSize(size_[0], size_[1]);

    for (auto size = size_, nSize = reduceSize(size);
         nSize >= cutOff && !(size[0] == 1 && size[1] == 1);
         size = nSize, nSize = reduceSize(size)) {

        drawTexId = !drawTexId;

        glDrawBuffer(GL_COLOR_ATTACHMENT0 + drawTexId);
        glBindTexture(GL_TEXTURE_2D,  textures_[!drawTexId]);

        sumProg_.setTextureUnit(0);
        sumProg_.setTextureSizeLimit(size[0], size[1]);

        quadDrawer_.setSize(nSize[0], nSize[1]);
        quadDrawer_.draw();
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return drawTexId;
}

uint64_t SumComputer::readSum(const std::array<int, 2> &cutOff, int storeTexId)
{
    uint64_t result = 0;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + storeTexId);

    if (useRGBA16FPacked_) {
        GLfloat pixels[4 * cutOff[0] * cutOff[1]];
        glReadPixels(0, 0, cutOff[0], cutOff[1], GL_RGBA, GL_FLOAT, &pixels);
        for (int p = 0; p < cutOff[0] * cutOff[1]; p++) {
            // unpack
            for (int j = 0; j < 4; j++) {
                uint64_t tmp = (int)(pixels[p * 4 + j]);
                for (int i = 1; i <= j; i++) {
                    tmp *= 2048;
                }
                result += tmp;
            }
        }
    } else {
        GLfloat pixels[cutOff[0] * cutOff[1]];
        glReadPixels(0, 0, cutOff[0], cutOff[1], GL_RED, GL_FLOAT, &pixels);
        for (int p = 0; p < cutOff[0] * cutOff[1]; p++) {
            result += (int)(pixels[p]);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result;
}

uint64_t SumComputer::sum(GLuint texture)
{
    return applyProgramAndSum(sadDimCutOff_, texNormalizeProg_, texture);
}

void SumComputer::storedBegin()
{
    storedIdx_ = 0;
}

void SumComputer::storedNext(GLuint textureA,
                             GLuint textureB,
                             const std::array<int, 2> &cutOff,
                             Program &diffProg)
{
    int drawTexId = kTexSumFirst;
    applyProgram(diffProg, textureA, drawTexId, textureB);
    drawTexId = reduceSum(cutOff);

    auto coo = getStoredCoo(size_, cutOff, storedIdx_);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);

    glDrawBuffer(GL_COLOR_ATTACHMENT0 + kTexStore);

    glBindTexture(GL_TEXTURE_2D, textures_[drawTexId]);

    glUseProgram(storeSumProg_.programId());

    storeSumProg_.setTextureUnit(0);
    storeSumProg_.setTextureSize(size_[0], size_[1]);
    storeSumProg_.setTextureSizeLimit(coo[0], coo[1]); // here, used to store origin

    quadDrawer_.setRect(coo[0], coo[1], cutOff[0], cutOff[1]);
    quadDrawer_.draw();

    glUseProgram(0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    storedIdx_++;
}

std::vector<uint64_t> SumComputer::storedEnd(const std::array<int, 2> &cutOff)
{
    std::vector<uint64_t> results(storedIdx_, 0);
    auto boundedSize = getStoredBoundedSize(size_, cutOff, storedIdx_);
    GLfloat pixels[(useRGBA16FPacked_ ? 4 : 1) * boundedSize[0] * boundedSize[1]];

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + kTexStore);

    glReadPixels(0, 0, boundedSize[0], boundedSize[1],
            useRGBA16FPacked_ ? GL_RGBA : GL_RED, GL_FLOAT, &pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (int idx = 0; idx < storedIdx_; idx++) {
        uint64_t result = 0;
        auto coo = getStoredCoo(size_, cutOff, idx);
        for (int i = coo[0]; i < coo[2]; i++) {
            for (int j = coo[1]; j < coo[3]; j++) {
                if (useRGBA16FPacked_) {  // unpack
                    for (int k = 0; k < 4; k++) {
                        uint64_t tmp = (int)(pixels[(i + j * boundedSize[0]) * 4 + k]);
                        for (int l = 1; l <= k; l++) {
                            tmp *= 2048;
                        }
                        result += tmp;
                    }
                } else {
                    result += (int)(pixels[i + j * boundedSize[0]]);
                }
            }
        }
        results[idx] = result;
    }

    return results;
}

void SumComputer::setSdMethod(SumComputer::Method method)
{
    method_ = method;
}

bool SumComputer::useSsd() const
{
    return method_ == SumComputer::Method::SSD;
}

uint64_t SumComputer::maxSd(GLuint texture)
{
    return applyProgramAndSum(useSsd() ? ssdDimCutOff_ : sadDimCutOff_,
                              useSsd() ? maxSquaredDiffProg_ : maxAbsDiffProg_,
                              texture);
}

uint64_t SumComputer::sd(GLuint textureA, GLuint textureB)
{
    return applyProgramAndSum(useSsd() ? ssdDimCutOff_ : sadDimCutOff_,
                              useSsd() ? squaredDiffProg_ : absDiffProg_,
                              textureA, textureB);
}

void SumComputer::sdStoredBegin()
{
    return storedBegin();
}

void SumComputer::sdStoredNext(GLuint textureA, GLuint textureB)
{
    storedNext(textureA, textureB,
               useSsd() ? ssdDimCutOff_ : sadDimCutOff_,
               useSsd() ? squaredDiffProg_ : absDiffProg_);
}

std::vector<uint64_t> SumComputer::sdStoredEnd()
{
    return storedEnd(useSsd() ? ssdDimCutOff_ : sadDimCutOff_);
}

int SumComputer::sdMaxStorable() const
{
    return useSsd() ? ssdMaxStorable_ : sadMaxStorable_;
}
