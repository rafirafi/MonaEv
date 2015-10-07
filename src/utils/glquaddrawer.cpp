#include "glquaddrawer.h"

QuadDrawer::QuadDrawer(int width, int height)
{
    init(width, height);
}

QuadDrawer::~QuadDrawer()
{
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}

void QuadDrawer::init(int width, int height)
{
    width_ = width;
    height_ = height;

    std::array<uint16_t, 8> verts{{
            0,        0,
            0,        height_,
            width_,   0,
            width_,   height_
            }};

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(verts[0]),
                 verts.data(),
                GL_STREAM_DRAW);

    glEnableVertexAttribArray(ShName::kACOO2D);

    glVertexAttribPointer(ShName::kACOO2D, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QuadDrawer::setSize(int width, int height)
{
    setRect(0, 0, width, height);
}

void QuadDrawer::setRect(int x, int y, int width, int height)
{
    if (width_ == width && height_ == height && x_ == x && y_ == y) {
        return;
    }

    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    std::array<uint16_t, 8> verts{{
            x_,                         y_,
            x_,                         (uint16_t)(y_ + height_),
            (uint16_t)(x_ + width_),    y_,
            (uint16_t)(x_ + width_),    (uint16_t)(y_ + height_)
            }};

    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(verts[0]),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    verts.size() * sizeof(verts[0]),
                    verts.data());

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QuadDrawer::draw() const
{
    glBindVertexArray(vao_);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
}
