#ifndef SUMCOMPUTER_H
#define SUMCOMPUTER_H

#include <stdint.h>

#include <vector>
#include <array>

#include <GL/glew.h>

#include "glquaddrawer.h"
#include "glutils.h"

class SumComputer
{
public:
    SumComputer()=default;
    ~SumComputer();

    enum Method {
        SAD = 0,
        SSD = 1
    };

    bool init(const std::array<int, 2> &size, bool forceRGBA16FPacked = false);

    uint64_t sum(GLuint texture);

    void setSdMethod(Method method);

    uint64_t maxSd(GLuint texture);

    uint64_t sd(GLuint textureA, GLuint textureB);

    void sdStoredBegin();

    void sdStoredNext(GLuint textureA, GLuint textureB);

    std::vector<uint64_t> sdStoredEnd();

    int sdMaxStorable() const;

private:
    std::array<int, 2> size_ = {{ 0, 0 }};

    GLuint fbo_ = 0;
    enum {
        kTexSumFirst = 0,
        kTexSumSecond,
        kTexStore,
        kTexMax
    };
    std::array<GLuint, kTexMax> textures_;

    QuadDrawer quadDrawer_;

    Program texNormalizeProg_;
    Program sumProg_;
    Program storeSumProg_;
    Program absDiffProg_;
    Program squaredDiffProg_;
    Program maxAbsDiffProg_;
    Program maxSquaredDiffProg_;

    bool useRGBA16FPacked_ = false;
    Method method_ = Method::SAD;

    std::array<int, 2> sadDimCutOff_ = {{ 1, 1}};
    std::array<int, 2> ssdDimCutOff_ = {{ 1, 1}};

    // deferred result
    int storedIdx_ = 0;
    int sadMaxStorable_ = 0;
    int ssdMaxStorable_ = 0;

    // convert to internal format
    void sumAndStore(GLuint texture);

    void applyProgram(Program &prog, GLuint textureA, int drawBufIdx = 0, GLuint textureB = 0);

    int reduceSum(const std::array<int, 2> &cutOff);

    uint64_t readSum(const std::array<int, 2> &cutOff, int storeTexId);

    uint64_t applyProgramAndSum(std::array<int, 2> &cutOff,
                                Program &prog,
                                GLuint textureA,
                                GLuint textureB = 0);

    void storedBegin();

    void storedNext(GLuint textureA,
                    GLuint textureB,
                    const std::array<int, 2> &cutOff,
                    Program &diffProg);

    std::vector<uint64_t> storedEnd(const std::array<int, 2> &cutOff);

    bool useSsd() const;
};

#endif // SUMCOMPUTER_H
