#ifndef GLIMAGELOADER_H
#define GLIMAGELOADER_H

#include <string>
#include <GL/glew.h>

GLuint loadImage(const std::string &filename, int &width, int &height);

#endif // GLIMAGELOADER_H
