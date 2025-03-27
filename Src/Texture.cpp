#include "../Headers/Texture.h"

Texture::Texture() {
    glGenTextures(1, &texture);
}

Texture::Texture(const char* filePath, bool rgba, int position) {
    glGenTextures(1, &texture);
    glActiveTexture(position);
    this->Bind();
    this->Load(filePath, rgba);
}

void Texture::Load(const char* filePath, bool rgba) {

    textureData = stbi_load(filePath, &width, &height, &nrChannels, 0);

    if (textureData) {
        if (rgba)
		    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
        else
		    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

}

void Texture::Unload() {
    stbi_image_free(textureData);
}

void Texture::Bind() {
    glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
