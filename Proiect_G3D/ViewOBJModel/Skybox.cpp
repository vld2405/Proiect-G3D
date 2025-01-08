#include "Skybox.h"

void Skybox::setupSkybox()
{
    float skyboxVertices[] = {
        // Positions            // Texture Coords // Normals
        // Back face
        -1.0f,  1.0f, -1.0f,   0.0f, 1.0f,       0.0f,  0.0f, -1.0f,  // 0
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f,       0.0f,  0.0f, -1.0f,  // 1
         1.0f, -1.0f, -1.0f,   1.0f, 0.0f,       0.0f,  0.0f, -1.0f,  // 2
         1.0f,  1.0f, -1.0f,   1.0f, 1.0f,       0.0f,  0.0f, -1.0f,  // 3

         // Front face
         -1.0f,  1.0f,  1.0f,   0.0f, 1.0f,       0.0f,  0.0f,  1.0f,  // 4
         -1.0f, -1.0f,  1.0f,   0.0f, 0.0f,       0.0f,  0.0f,  1.0f,  // 5
          1.0f, -1.0f,  1.0f,   1.0f, 0.0f,       0.0f,  0.0f,  1.0f,  // 6
          1.0f,  1.0f,  1.0f,   1.0f, 1.0f,       0.0f,  0.0f,  1.0f,  // 7

          // Left face
          -1.0f,  1.0f,  1.0f,   0.0f, 1.0f,      -1.0f,  0.0f,  0.0f,  // 8
          -1.0f, -1.0f,  1.0f,   0.0f, 0.0f,      -1.0f,  0.0f,  0.0f,  // 9
          -1.0f, -1.0f, -1.0f,   1.0f, 0.0f,      -1.0f,  0.0f,  0.0f,  // 10
          -1.0f,  1.0f, -1.0f,   1.0f, 1.0f,      -1.0f,  0.0f,  0.0f,  // 11

          // Right face
           1.0f,  1.0f,  1.0f,   0.0f, 1.0f,       1.0f,  0.0f,  0.0f,  // 12
           1.0f, -1.0f,  1.0f,   0.0f, 0.0f,       1.0f,  0.0f,  0.0f,  // 13
           1.0f, -1.0f, -1.0f,   1.0f, 0.0f,       1.0f,  0.0f,  0.0f,  // 14
           1.0f,  1.0f, -1.0f,   1.0f, 1.0f,       1.0f,  0.0f,  0.0f,  // 15

           // Top face
           -1.0f,  1.0f,  1.0f,   0.0f, 1.0f,       0.0f,  1.0f,  0.0f,  // 16
           -1.0f,  1.0f, -1.0f,   0.0f, 0.0f,       0.0f,  1.0f,  0.0f,  // 17
            1.0f,  1.0f, -1.0f,   1.0f, 0.0f,       0.0f,  1.0f,  0.0f,  // 18
            1.0f,  1.0f,  1.0f,   1.0f, 1.0f,       0.0f,  1.0f,  0.0f,  // 19

            // Bottom face
            -1.0f, -1.0f,  1.0f,   0.0f, 1.0f,       0.0f, -1.0f,  0.0f,  // 20
            -1.0f, -1.0f, -1.0f,   0.0f, 0.0f,       0.0f, -1.0f,  0.0f,  // 21
             1.0f, -1.0f, -1.0f,   1.0f, 0.0f,       0.0f, -1.0f,  0.0f,  // 22
             1.0f, -1.0f,  1.0f,   1.0f, 1.0f,       0.0f, -1.0f,  0.0f,  // 23
    };


    unsigned int skyboxIndices[] = {
        // Back face
        0, 1, 2, 2, 3, 0,
        // Front face
        4, 5, 6, 6, 7, 4,
        // Left face
        4, 5, 1, 1, 0, 4,
        // Right face
        3, 2, 6, 6, 7, 3,
        // Top face
        4, 0, 3, 3, 7, 4,
        // Bottom face
        1, 5, 6, 6, 2, 1
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute (3 floats)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Skybox::loadTextures(const std::vector<std::string>& skyPaths) {
    glGenTextures(1, &textureID);  // Generate texture ID for cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);  // Bind cubemap texture

    int width, height, nrChannels;

    // Iterate over each face of the cubemap
    for (unsigned int i = 0; i < skyPaths.size(); ++i) {
        unsigned char* data = stbi_load(skyPaths[i].c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            // Load texture for each face
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Failed to load cubemap texture at path: " << skyPaths[i] << std::endl;
            stbi_image_free(data); // Ensure memory is freed even if loading fails
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Unbind the cubemap texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


Skybox::Skybox(const std::vector<std::string>& skyPaths)
{
    setupSkybox();
    loadTextures(skyPaths);
}

Skybox::~Skybox()
{
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Skybox::draw(Shader& shader)
{
    // Save the current depth function so we can reset it after drawing the skybox
    glDepthFunc(GL_LEQUAL); // Change depth function to allow rendering the skybox behind everything

    glBindVertexArray(VAO);
    // Bind the cube map texture
    glActiveTexture(GL_TEXTURE0);  // Activate the texture unit 0
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    shader.setInt("skybox", 0);
    // Bind the VAO of the skybox and draw it

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Unbind the VAO
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS);
}

