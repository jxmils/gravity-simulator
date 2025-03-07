#pragma once

#include <glad/glad.h>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For translate, scale, etc.
#include <glm/gtc/type_ptr.hpp>         // For value_ptr

class Shader;  // Forward declaration

class CelestialBody {
public:
    float x, y, vx, vy, mass, radius;
    float color[3];
    mutable unsigned int VAO, VBO;  // Made mutable since they need to be modified in const methods
    mutable GLint modelLoc, colorLoc;  // Made mutable since they need to be modified in const methods
    mutable int vertexCount;  // Made mutable since it needs to be modified in const methods

    CelestialBody(float x, float y, float vx, float vy, float mass, float radius, float r, float g, float b);
    ~CelestialBody();

    void computeAcceleration(float& ax, float& ay);
    void updatePosition(float deltaTime = 1.0f/60.0f);  // Default to 60 FPS if not specified
    void draw(const Shader& shader) const;  // Made const since it doesn't modify object state
    void initializeBuffers() const;  // Made const since it's called from const methods
    void setupShaderUniforms(const Shader& shader) const;  // Made const since it's called from const draw

private:
    void cleanup();  // Helper method to clean up OpenGL resources
    const float G = 6.67430e-11f;  // Gravitational constant
};