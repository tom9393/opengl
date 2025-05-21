#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "stb_image.h"

class OldTexture
{
    public:
    unsigned int texture;
    unsigned char* textureData;
    int width;
    int height;
    int nrChannels;

    OldTexture();
    OldTexture(const char* filePath, bool rgba, int position);
    void Load(const char* filePath, bool rgba);
    void Unload();
    void Bind();
    void Unbind();

};

#endif
