#include "SpacetimeGrid.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

SpacetimeGrid::SpacetimeGrid() : VAO(0), VBO(0) {
    std::cout << "Starting SpacetimeGrid initialization..." << std::endl;
    try {
        // Generate grid vertices
        for (int i = -20; i <= 20; i++) {
            float x = i / 20.0f;
            // Vertical lines
            vertices.push_back(-1.0f);
            vertices.push_back(x);
            vertices.push_back(1.0f);
            vertices.push_back(x);
            
            // Horizontal lines
            vertices.push_back(x);
            vertices.push_back(-1.0f);
            vertices.push_back(x);
            vertices.push_back(1.0f);
        }
        std::cout << "Grid vertices generated: " << vertices.size() << " vertices" << std::endl;
        
        initializeBuffers();
        std::cout << "SpacetimeGrid initialization completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in SpacetimeGrid initialization: " << e.what() << std::endl;
        throw;
    }
}

SpacetimeGrid::~SpacetimeGrid() {
    std::cout << "SpacetimeGrid cleanup starting..." << std::endl;
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        std::cout << "VAO deleted" << std::endl;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        std::cout << "VBO deleted" << std::endl;
    }
    std::cout << "SpacetimeGrid cleanup completed" << std::endl;
}

void SpacetimeGrid::initializeBuffers() {
    std::cout << "Initializing SpacetimeGrid buffers..." << std::endl;
    
    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    if (VAO == 0) {
        throw std::runtime_error("Failed to generate VAO");
    }
    std::cout << "VAO generated: " << VAO << std::endl;
    
    glGenBuffers(1, &VBO);
    if (VBO == 0) {
        throw std::runtime_error("Failed to generate VBO");
    }
    std::cout << "VBO generated: " << VBO << std::endl;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in initializeBuffers: 0x" << std::hex << err << std::dec << std::endl;
    }
    
    std::cout << "Buffer initialization completed" << std::endl;
}

void SpacetimeGrid::drawGrid(const Shader& shader, float time) {
    shader.use();
    
    // Set time uniform
    shader.setFloat("time", time);
    
    if (VAO == 0) {
        std::cerr << "Error: Attempting to draw with invalid VAO" << std::endl;
        return;
    }
    
    glBindVertexArray(VAO);
    
    // Draw all the lines
    glDrawArrays(GL_LINES, 0, vertices.size() / 2);
    
    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in drawGrid: 0x" << std::hex << err << std::dec << std::endl;
    }
}

float SpacetimeGrid::calculateWarp(float x, float y, const std::vector<CelestialBody>& bodies) {
    float warp = 0.0f;
    for (const auto& body : bodies) {
        float dx = x - body.x;
        float dy = y - body.y;
        float r = sqrt(dx*dx + dy*dy);
        if (r > 0.05f) {  // Avoid division by zero or singularities
            warp += -0.02f * (body.mass) / (r); // Simplified warp calculation
        }
    }
    return warp;
}