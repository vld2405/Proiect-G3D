#include "CTexture.h"
#include <iostream>
#include <stb_image.h> 

void CTexture::loadTexture2D(const std::string& filePath) {
    std::cout << "Loading texture: " << filePath << std::endl;

    // Step 1: Create texture ID and bind the texture
    glGenTextures(1, &textureID);
    if (textureID == 0) {
        std::cerr << "Failed to generate texture ID for: " << filePath << std::endl;
        return;
    }
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Step 2: Load the texture data using stb_image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Failed to load texture image: " << filePath << std::endl;
        return;
    }

    std::cout << "Texture loaded successfully: " << filePath
        << " Width: " << width
        << " Height: " << height
        << " Channels: " << nrChannels << std::endl;

    // Step 3: Determine the format based on channels
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    // Step 4: Upload texture to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Step 5: Set texture parameters to enable mipmap generation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Check if texture is properly bound
    GLint boundTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
    if (boundTexture != textureID) {
        std::cerr << "Error: Texture not correctly bound before mipmap generation!" << std::endl;
    }

    // Step 6: Generate mipmaps if texture is successfully created
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error before mipmap generation: " << error << std::endl;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after mipmap generation: " << error << std::endl;
    }
    else {
        std::cout << "Mipmaps generated successfully for: " << filePath << std::endl;
    }

    // Step 7: Free the texture image data after it's uploaded
    stbi_image_free(data);
    std::cout << "Texture " << filePath << " loaded and memory freed." << std::endl;
}

void CTexture::bindTexture() {
    if (textureID == 0) {
        std::cerr << "Error: Attempting to bind a texture with invalid texture ID." << std::endl;
        return;
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    std::cout << "Texture " << textureID << " bound successfully." << std::endl;
}

void CTexture::setFiltering(int magFilter, int minFilter) {
    if (textureID == 0) {
        std::cerr << "Error: Attempting to set filtering for invalid texture ID." << std::endl;
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    std::cout << "Texture filtering set. Mag Filter: " << magFilter << " Min Filter: " << minFilter << std::endl;
}

void CTexture::setSamplerParameter(GLenum param, GLenum value) {
    if (textureID == 0) {
        std::cerr << "Error: Attempting to set sampler parameter for invalid texture ID." << std::endl;
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, param, value);
    std::cout << "Sampler parameter set: " << param << " = " << value << std::endl;
}

void CTexture::releaseTexture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        std::cout << "Texture " << textureID << " released." << std::endl;
        textureID = 0;
    }
    else {
        std::cerr << "Error: Attempting to release an already released or uninitialized texture." << std::endl;
    }
}

GLuint CTexture::getTextureID() const {
    return textureID;  // Return the texture ID
}
