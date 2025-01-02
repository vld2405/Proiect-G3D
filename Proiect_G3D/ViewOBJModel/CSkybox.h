#ifndef CSKYBOX_H
#define CSKYBOX_H

#include <GL/glew.h>
#include <string>
#include "CVertexBufferObject.h"
#include "CTexture.h"
#include <glm/glm.hpp>

#define TEXTURE_FILTER_MAG_BILINEAR 1
#define TEXTURE_FILTER_MIN_BILINEAR 2


class CSkybox {
public:
    void loadSkybox(const std::string& a_sDirectory,
        const std::string& a_sFront, const std::string& a_sBack,
        const std::string& a_sLeft, const std::string& a_sRight,
        const std::string& a_sTop, const std::string& a_sBottom);

    void renderSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void releaseSkybox();

private:
    GLuint uiVAO;
    CVertexBufferObject vboRenderData;
    GLuint shaderProgram;

    CTexture tTextures[6];
    std::string sDirectory;
    std::string sFront, sBack, sLeft, sRight, sTop, sBottom;

    GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource);
};

#endif

