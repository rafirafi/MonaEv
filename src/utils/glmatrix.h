#ifndef GLMATRIX_H
#define GLMATRIX_H

#include <array>

#include <GL/glew.h>

using Matrix4=std::array<GLfloat, 16>;

Matrix4 gldLoadIdentity();

Matrix4 gldOrtho(const Matrix4 &m,
                           float left,
                           float right,
                           float bottom,
                           float top,
                           float znear,
                           float zfar);

#endif // GLMATRIX_H
