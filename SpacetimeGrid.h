#ifndef SPACETIMEGRID_H
#define SPACETIMEGRID_H

#include "CelestialBody.h"
#include "Shader.h"
#include <vector>
#include <glad/glad.h>

class SpacetimeGrid {
private:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    GLint timeLoc;  // Cache the time uniform location
    void initializeBuffers();
    void setupShaderUniforms(const Shader& shader);  // New method to setup uniforms
    void cleanup();  // Helper method to clean up OpenGL resources

public:
    SpacetimeGrid();
    ~SpacetimeGrid();
    
    // Draws a grid warped by gravitational influences.
    // Now takes a Shader reference for rendering and a time value for dynamic effects.
    void drawGrid(const Shader& shader, float time);

    // Calculates the warp (vertical displacement) at a grid point (x, y)
    // based on the gravitational effect of the provided bodies.
    float calculateWarp(float x, float y, const std::vector<CelestialBody>& bodies);
};

#endif // SPACETIMEGRID_H