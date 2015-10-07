#include "glutils.h"

#include <map>

GLuint prepareFbo(int width,
                  int height,
                  GLint textureInternalFormat,
                  GLenum textureFormat,
                  GLenum textureType,
                  GLuint *textures,
                  int texturesSize)
{
    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    // TODO: if OGL 2.1 check glGenFramebuffersEXT


    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glGenTextures(texturesSize, textures);

    for (unsigned int i = 0, iend = texturesSize; i < iend; i++) {

        glBindTexture(GL_TEXTURE_2D, textures[i]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     textureInternalFormat,
                     width,
                     height,
                     0,
                     textureFormat,
                     textureType,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D,
                               textures[i],
                               0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // asked by some OGL implementation
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLuint test = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (test != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "%s Failed to create texture render target %x\n", __func__, test);
        if (test != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "Could not create framebuffer object for rendering\n");
        }
        glDeleteFramebuffers(1, &fboId);
    } else {
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        for (unsigned int i = 0, iend = texturesSize; i < iend; i++) {
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        glDrawBuffer(GL_NONE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fboId;
}

// Global variable
static std::map<int, std::string> g_attMap = {
    {ShName::kACOO2D,            "coord2d"},
    {ShName::kACOLOR,            "acolor"},
    {ShName::kMVP,               "mvp"},
    {ShName::kTEX_UNIT0,         "texUnit0"},
    {ShName::kTEX_UNIT1,         "texUnit1"},
    {ShName::kTEX_SIZE,          "texSize"},
    {ShName::kTEX_SIZE_LIM,    "texSizeLimit"}
};

// https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/VideoBackends/OGL/GLUtil.cpp
static GLuint compileProgram(const char* vertexShader, const char* fragmentShader)
{
    // generate objects
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint programID = glCreateProgram();

    // compile vertex shader
    glShaderSource(vertexShaderID, 1, &vertexShader, nullptr);
    glCompileShader(vertexShaderID);
#if !defined(NDEBUG)
    GLint result = GL_FALSE;
    char stringBuffer[1024];
    GLsizei stringBufferUsage = 0;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderInfoLog(vertexShaderID, 1024, &stringBufferUsage, stringBuffer);

    /*if (result && stringBufferUsage)
    {
        fprintf(stderr, "GLSL vertex shader warnings:\n%s%s", stringBuffer, vertexShader);
    }
    else*/ if (!result)
    {
        fprintf(stderr, "GLSL vertex shader error:\n%s%s", stringBuffer, vertexShader);
        return 0;
    }

    bool shader_errors = !result;
#endif

    // compile fragment shader
    glShaderSource(fragmentShaderID, 1, &fragmentShader, nullptr);
    glCompileShader(fragmentShaderID);
#if !defined(NDEBUG)
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderInfoLog(fragmentShaderID, 1024, &stringBufferUsage, stringBuffer);

    /*if (result && stringBufferUsage)
    {
        fprintf(stderr, "GLSL fragment shader warnings:\n%s%s", stringBuffer, fragmentShader);
    }
    else*/ if (!result)
    {
        fprintf(stderr, "GLSL fragment shader error:\n%s%s", stringBuffer, fragmentShader);
        return 0;
    }

    shader_errors |= !result;
#endif

    // link them
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);

    // RAFI: set our attributes
    glBindAttribLocation(programID, ShName::kACOO2D, g_attMap[ShName::kACOO2D].data());
    glBindAttribLocation(programID, ShName::kACOLOR, g_attMap[ShName::kACOLOR].data());

    glLinkProgram(programID);

#if !defined(NDEBUG)
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramInfoLog(programID, 1024, &stringBufferUsage, stringBuffer);

   /* if (result && stringBufferUsage)
    {
        fprintf(stderr, "GLSL linker warnings:\n%s%s%s", stringBuffer, vertexShader, fragmentShader);
    }
    else*/ if (!result && !shader_errors)
    {
        fprintf(stderr, "GLSL linker error:\n%s%s%s", stringBuffer, vertexShader, fragmentShader);
        return 0;
    }
#endif

    // cleanup
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

Program::Program() :
    programId_(0), mvp_(gldLoadIdentity())
{
}

Program::Program(const char *vert, const char *frag, int width, int height) :
    programId_(0), mvp_(gldLoadIdentity())
{
    init(vert, frag, width, height);
}

bool Program::init(const char *vert, const char *frag, int width, int height)
{
    programId_ = compileProgram(vert, frag);
    if (programId_ == 0) {
        return false;
    }

    uniform_mvp_ = glGetUniformLocation(programId_, g_attMap[ShName::kMVP].data());

    uniform_texUnit1_ = glGetUniformLocation(programId_, g_attMap[ShName::kTEX_UNIT0].data());
    uniform_texUnit2_ = glGetUniformLocation(programId_, g_attMap[ShName::kTEX_UNIT1].data());

    uniform_texSize_ = glGetUniformLocation(programId_, g_attMap[ShName::kTEX_SIZE].data());

    uniform_texSizeLim_ = glGetUniformLocation(programId_, g_attMap[ShName::kTEX_SIZE_LIM].data());

    if (width && height) {
        glUseProgram(programId_);
        setOrtho(width, height);
        glUseProgram(0);
    }

    return true;
}

Program::~Program()
{
    glUseProgram(0);
    glDeleteProgram(programId_);
}

GLuint Program::programId() const
{
    return programId_;
}


/* Using coordinates with bottom left origin except shapes drawing which is top left origin
 * to mimic common vector 2D graphic libraries.
*/
void Program::setOrtho(int width, int height)
{
    if (height < 0) {
        mvp_ = gldOrtho(gldLoadIdentity(), 0, width, -height, 0, -1, 1);
    } else {
        mvp_ = gldOrtho(gldLoadIdentity(), 0, width, 0, height, -1, 1);
    }
    glUniformMatrix4fv(uniform_mvp_, 1, GL_FALSE, &mvp_[0]);
}

void Program::setTextureUnit(const GLuint &unit, int unitIdx)
{
    glUniform1i(unitIdx == 0 ? uniform_texUnit1_ : uniform_texUnit2_, static_cast<int>(unit));
}

void Program::setTextureSize(int width, int height)
{
    glUniform2f(uniform_texSize_, (float)width, (float)height);
}

void Program::setTextureSizeLimit(int width, int height)
{
    glUniform2f(uniform_texSizeLim_, (float)width, (float)height);
}

