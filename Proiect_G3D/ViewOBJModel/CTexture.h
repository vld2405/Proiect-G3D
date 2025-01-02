#ifndef CTEXTURE_H
#define CTEXTURE_H

#include <GL/glew.h>
#include <string>

class CTexture {
public:
    void loadTexture2D(const std::string& filePath);
    void bindTexture();
    void setFiltering(int magFilter, int minFilter);
    void setSamplerParameter(GLenum param, GLenum value);
    void releaseTexture();
    GLuint getTextureID() const;


private:
    GLuint textureID;
};

#endif
