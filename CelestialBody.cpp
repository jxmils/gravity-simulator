#include "CelestialBody.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

// Constants
const float G = 6.67430e-11;
const float c = 3e8;
const float dt = 0.0005f;

CelestialBody::CelestialBody(float x, float y, float vx, float vy, float mass, float radius, float r, float g, float b)
    : x(x), y(y), vx(vx), vy(vy), mass(mass), radius(radius), VAO(0), VBO(0), modelLoc(-1), colorLoc(-1) {
    color[0] = r; color[1] = g; color[2] = b;
    initializeBuffers();
}

CelestialBody::~CelestialBody() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
}

void CelestialBody::initializeBuffers() {
    std::vector<float> vertices;
    vertices.push_back(0.0f); // Center point
    vertices.push_back(0.0f);

    // Generate circle vertices
    const int segments = 50;
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        vertices.push_back(cosf(theta));
        vertices.push_back(sinf(theta));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in initializeBuffers: " << err << std::endl;
    }
}

void CelestialBody::setupShaderUniforms(const Shader& shader) {
    modelLoc = glGetUniformLocation(shader.ID, "model");
    colorLoc = glGetUniformLocation(shader.ID, "color");
    
    if (modelLoc == -1) {
        std::cerr << "Warning: 'model' uniform not found in shader" << std::endl;
    }
    if (colorLoc == -1) {
        std::cerr << "Warning: 'color' uniform not found in shader" << std::endl;
    }
}

void CelestialBody::computeAcceleration(float& ax, float& ay) {
    float r = sqrt(x*x + y*y);
    float r3 = r * r * r;
    float factor = -G * mass / r3;
    float relativistic_correction = 1 + (3 * G * mass) / (c * c * r);
    ax = factor * x * relativistic_correction;
    ay = factor * y * relativistic_correction;
}

void CelestialBody::updatePosition() {
    float k1x = vx * dt, k1y = vy * dt;
    float k1vx, k1vy;
    computeAcceleration(k1vx, k1vy);
    k1vx *= dt; k1vy *= dt;

    float k2x = (vx + 0.5 * k1vx) * dt, k2y = (vy + 0.5 * k1vy) * dt;
    float k2vx, k2vy;
    computeAcceleration(k2vx, k2vy);
    k2vx *= dt; k2vy *= dt;

    float k3x = (vx + 0.5 * k2vx) * dt, k3y = (vy + 0.5 * k2vy) * dt;
    float k3vx, k3vy;
    computeAcceleration(k3vx, k3vy);
    k3vx *= dt; k3vy *= dt;

    float k4x = (vx + k3vx) * dt, k4y = (vy + k3vy) * dt;
    float k4vx, k4vy;
    computeAcceleration(k4vx, k4vy);
    k4vx *= dt; k4vy *= dt;

    x += (k1x + 2 * k2x + 2 * k3x + k4x) / 6;
    y += (k1y + 2 * k2y + 2 * k3y + k4y) / 6;
    vx += (k1vx + 2 * k2vx + 2 * k3vx + k4vx) / 6;
    vy += (k1vy + 2 * k2vy + 2 * k3vy + k4vy) / 6;
}

void CelestialBody::draw(const Shader& shader) {
    shader.use();
    
    if (modelLoc == -1 || colorLoc == -1) {
        setupShaderUniforms(shader);
    }

    glBindVertexArray(VAO);
    
    // Create model matrix for position and scale (column-major order)
    float modelMatrix[16] = {
        radius, 0.0f, 0.0f, 0.0f,   // First column
        0.0f, radius, 0.0f, 0.0f,   // Second column
        0.0f, 0.0f, 1.0f, 0.0f,     // Third column
        x, y, 0.0f, 1.0f            // Fourth column (translation)
    };
    
    // Set uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);  // GL_FALSE for column-major
    glUniform3fv(colorLoc, 1, color);

    // Draw the celestial body
    glDrawArrays(GL_TRIANGLE_FAN, 0, 52); // 50 segments + center point + closing point
    
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in draw: " << err << std::endl;
    }
}