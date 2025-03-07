#pragma once

#include <glad/glad.h>
#include <cmath>
#include <vector>

class Shader;  // Forward declaration

class CelestialBody {
public:
    float x, y, vx, vy, mass, radius;
    float color[3];
    unsigned int VAO, VBO;
    GLint modelLoc, colorLoc;  // Cache uniform locations
    int vertexCount;  // Number of vertices in the circle

    CelestialBody(float x, float y, float vx, float vy, float mass, float radius, float r, float g, float b);
    ~CelestialBody();

    void computeAcceleration(float& ax, float& ay);
    void updatePosition();
    void draw(const Shader& shader);  // Now takes shader as parameter
    void initializeBuffers();
    void setupShaderUniforms(const Shader& shader);  // New method to setup uniforms

private:
    void cleanup();  // Helper method to clean up OpenGL resources
};