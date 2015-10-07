#include "glmatrix.h"

static Matrix4 makeOrtho(GLfloat left,
                            GLfloat right,
                            GLfloat bottom,
                            GLfloat top,
                            GLfloat znear,
                            GLfloat zfar)
{
    if (left == right || bottom == top || znear == zfar) {
        return gldLoadIdentity();
    }

    GLfloat p_fn = zfar + znear;
    GLfloat m_nf = znear - zfar;  // ~ -m_fn

    GLfloat p_rl = right + left;
    GLfloat m_rl = right - left;
    GLfloat p_tb = top + bottom;
    GLfloat m_tb = top - bottom;

    GLfloat m_lr = -m_rl;
    GLfloat m_bt = -m_tb;

    return Matrix4{{
                (2.f)/m_rl,  (0.f),           (0.f),           (0.f)
                ,(0.f),       (2.f)/m_tb,      (0.f),           (0.f)
                ,(0.f),       (0.f),           (2.f)/m_nf,      (0.f)
                ,p_rl/m_lr,   p_tb/m_bt,        p_fn/m_nf,       (1.f)
                }};
}

static Matrix4 gldMultMatrix(const Matrix4 &B, const Matrix4 &A)
{
    Matrix4 matrix;

    for (unsigned int i = 0; i < 4; i++){  // Cycle through each vector of first matrix.
        matrix[i*4]   = A[i*4] * B[0] + A[i*4+1] * B[4] + A[i*4+2] * B[8]  + A[i*4+3] * B[12];
        matrix[i*4+1] = A[i*4] * B[1] + A[i*4+1] * B[5] + A[i*4+2] * B[9]  + A[i*4+3] * B[13];
        matrix[i*4+2] = A[i*4] * B[2] + A[i*4+1] * B[6] + A[i*4+2] * B[10] + A[i*4+3] * B[14];
        matrix[i*4+3] = A[i*4] * B[3] + A[i*4+1] * B[7] + A[i*4+2] * B[11] + A[i*4+3] * B[15];
    }

    return matrix;
}

Matrix4 gldLoadIdentity()
{
    return Matrix4{{
        (1.f),      (0.f),           (0.f),           (0.f)
        ,(0.f),      (1.f),           (0.f),           (0.f)
        ,(0.f),      (0.f),           (1.f),           (0.f)
        ,(0.f),      (0.f),           (0.f),           (1.f)
   }};
}

Matrix4 gldOrtho(const Matrix4 &m,
                           float left,
                           float right,
                           float bottom,
                           float top,
                           float znear,
                           float zfar)
{
    return gldMultMatrix(m, makeOrtho(left, right, bottom, top, znear, zfar));
}
