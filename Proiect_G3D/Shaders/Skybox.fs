#version 330 core
in vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;
uniform vec4 lightColor;

void main()
{
    FragColor = texture(skybox, TexCoords) * lightColor;
}
