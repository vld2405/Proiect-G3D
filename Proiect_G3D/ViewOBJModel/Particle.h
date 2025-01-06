#ifndef PARTICLE_H
#define PARTICLE_H

#include <GLM.hpp>

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float life;

    Particle() : position(0.0f), velocity(0.0f), life(0.0f) {}
};

#endif // PARTICLE_H
