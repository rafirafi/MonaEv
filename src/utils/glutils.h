#ifndef GLUTILS_H
#define GLUTILS_H

#include <array>

#include <GL/glew.h>

#include "glmatrix.h"

GLuint prepareFbo(int width,
                  int height,
                  GLint textureInternalFormat,
                  GLenum textureFormat,
                  GLenum textureType,
                  GLuint *textures,
                  int texturesSize);

namespace ShName
{
enum : int {
    // attrib
    kACOO2D = 0,
    kACOLOR,
    // uniform
    kMVP,
    kTEX_UNIT0,
    kTEX_UNIT1,
    kTEX_SIZE,
    kTEX_SIZE_LIM
};
}

class Program
{
public:
    Program();
    Program(const char *vert,
            const char *frag,
            int width = 0,
            int height = 0);

    ~Program();

    bool init(const char *vert, const char *frag, int width = 0, int height = 0);

    GLuint programId() const;

    void setTextureUnit(const GLuint &unit, int unitIdx = 0);

    void setTextureSize(int width, int height);

    void setTextureSizeLimit(int width, int height);

    void setOrtho(int width, int height);

private:
    GLuint programId_;

    Matrix4 mvp_;

    GLint uniform_mvp_ = -1;
    GLint uniform_texUnit1_ = -1;
    GLint uniform_texUnit2_ = -1;
    GLint uniform_texSize_ = -1;
    GLint uniform_texSizeLim_ = -1;
};

#endif // GLUTILS_H
