#ifndef GLQUADDRAWER_H
#define GLQUADDRAWER_H

#include "glutils.h"

#include <stdint.h>

class QuadDrawer
{
public:
    QuadDrawer()=default;
    QuadDrawer(int width, int height);
    ~QuadDrawer();

    void setSize(int width, int height);

    void setRect(int x, int y, int width, int height);

    void draw() const;

    void init(int width, int height);
private:
    uint16_t x_ = 0;
    uint16_t y_ = 0;
    uint16_t width_ = 0;
    uint16_t height_ = 0;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    void init();
};

#endif // GLQUADDRAWER_H
