#ifndef CELESTIALBODY_H
#define CELESTIALBODY_H

#include <glad/glad.h>
#include <cmath>

class Shader;  // Forward declaration

class CelestialBody {
public:
    float x, y, vx, vy, mass, radius;
    float color[3];
    unsigned int VAO, VBO;
    GLint modelLoc, colorLoc;  // Cache uniform locations

    CelestialBody(float x, float y, float vx, float vy, float mass, float radius, float r, float g, float b);
    ~CelestialBody();

    void computeAcceleration(float& ax, float& ay);
    void updatePosition();
    void draw(const Shader& shader);  // Now takes shader as parameter
    void initializeBuffers();
    void setupShaderUniforms(const Shader& shader);  // New method to setup uniforms
};

#endif