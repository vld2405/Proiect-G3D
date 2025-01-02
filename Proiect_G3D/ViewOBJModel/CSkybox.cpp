#include "CSkybox.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
out vec3 TexCoords;
uniform mat4 view;
uniform mat4 projection;
void main() {
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 TexCoords;
out vec4 FragColor;
uniform samplerCube skybox;
void main() {
    FragColor = texture(skybox, TexCoords);
}
)";

void CSkybox::loadSkybox(const std::string& a_sDirectory, const std::string& a_sFront, const std::string& a_sBack, const std::string& a_sLeft, const std::string& a_sRight, const std::string& a_sTop, const std::string& a_sBottom) {
    std::cerr << "Loading skybox textures..." << std::endl;
    tTextures[0].loadTexture2D(a_sDirectory + a_sFront);
    tTextures[1].loadTexture2D(a_sDirectory + a_sBack);
    tTextures[2].loadTexture2D(a_sDirectory + a_sLeft);
    tTextures[3].loadTexture2D(a_sDirectory + a_sRight);
    tTextures[4].loadTexture2D(a_sDirectory + a_sTop);
    tTextures[5].loadTexture2D(a_sDirectory + a_sBottom);

    std::cerr << "Setting texture parameters..." << std::endl;
    for (int i = 0; i < 6; ++i) {
        tTextures[i].setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR);
        tTextures[i].setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        tTextures[i].setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    std::cerr << "Generating vertex array..." << std::endl;
    glGenVertexArrays(1, &uiVAO);
    glBindVertexArray(uiVAO);

    std::cerr << "Creating and binding VBO..." << std::endl;
    vboRenderData.createVBO();
    vboRenderData.bindVBO();

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
    };

    std::cerr << "Adding data to VBO..." << std::endl;
    vboRenderData.addData(skyboxVertices, sizeof(skyboxVertices));
    vboRenderData.uploadDataToGPU(GL_STATIC_DRAW);

    std::cerr << "Setting up vertex attributes..." << std::endl;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    std::cerr << "Loading shader program..." << std::endl;
    shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
}

void CSkybox::renderSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    std::cerr << "Rendering skybox..." << std::endl;

    glDepthFunc(GL_LEQUAL);  // Ensure skybox renders behind other objects

    glUseProgram(shaderProgram);

    // Remove the translation component from the view matrix
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewNoTranslation[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glBindVertexArray(uiVAO);
    glActiveTexture(GL_TEXTURE0);

    // Bind each of the 6 skybox textures
    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tTextures[i].getTextureID());
    }

    glDrawArrays(GL_TRIANGLES, 0, 36);  // Render skybox

    glDepthFunc(GL_LESS);  // Reset depth function for other objects
}


void CSkybox::releaseSkybox() {
    std::cerr << "Releasing skybox resources..." << std::endl;
    vboRenderData.releaseVBO();
    glDeleteVertexArrays(1, &uiVAO);
    glDeleteProgram(shaderProgram);
    for (int i = 0; i < 6; ++i) {
        tTextures[i].releaseTexture();
    }
}

GLuint CSkybox::loadShader(const char* vertexShaderSource, const char* fragmentShaderSource) {
    std::cerr << "Loading shaders..." << std::endl;
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Error compiling vertex shader: " << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Error compiling fragment shader: " << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
