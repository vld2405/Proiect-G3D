#pragma once
#include <glm.hpp>
#include "stb_image.h"
#include "Model.h"   

class Skybox
{
private:
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    glm::vec3 position;
    glm::vec3 scale;
    void setupSkybox();
    void loadTextures(const std::vector<std::string>& skyPath);
public:
    Skybox(const std::vector<std::string>& skyPaths);
    ~Skybox();
    void draw(Shader& shader);
    unsigned int getTextureID() const { return textureID; }
};

