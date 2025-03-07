#include "SpacetimeGrid.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

SpacetimeGrid::SpacetimeGrid() : VAO(0), VBO(0), timeLoc(-1) {
    std::cout << "Starting SpacetimeGrid initialization..." << std::endl;
    
    // Check if we have a valid OpenGL context
    if (glGetString(GL_VERSION) == nullptr) {
        std::cerr << "No valid OpenGL context found!" << std::endl;
        throw std::runtime_error("No valid OpenGL context");
    }
    
    std::cout << "OpenGL Context Version: " << glGetString(GL_VERSION) << std::endl;
    
    try {
        // Generate grid vertices
        const int gridSize = 20;
        const float step = 2.0f / gridSize;  // Step size to cover -1 to 1
        
        // Pre-calculate the number of vertices needed
        int numVerticalLines = gridSize + 1;
        int numHorizontalLines = gridSize + 1;
        int totalVertices = (numVerticalLines + numHorizontalLines) * 2;  // 2 points per line
        vertices.reserve(totalVertices * 2);  // 2 floats per vertex
        
        std::cout << "Generating grid with " << numVerticalLines << " vertical and " 
                  << numHorizontalLines << " horizontal lines..." << std::endl;

        // Vertical lines
        for (int i = 0; i <= gridSize; i++) {
            float x = -1.0f + i * step;
            vertices.push_back(x);      // Start point x
            vertices.push_back(-1.0f);  // Start point y
            vertices.push_back(x);      // End point x
            vertices.push_back(1.0f);   // End point y
        }

        // Horizontal lines
        for (int i = 0; i <= gridSize; i++) {
            float y = -1.0f + i * step;
            vertices.push_back(-1.0f);  // Start point x
            vertices.push_back(y);      // Start point y
            vertices.push_back(1.0f);   // End point x
            vertices.push_back(y);      // End point y
        }

        std::cout << "Grid vertices generated: " << vertices.size() / 2 << " vertices" << std::endl;
        
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
    
    // Check if we have a valid OpenGL context
    if (glGetString(GL_VERSION) == nullptr) {
        std::cerr << "No valid OpenGL context found during buffer initialization!" << std::endl;
        throw std::runtime_error("No valid OpenGL context");
    }
    
    // Get max vertex attribs
    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    std::cout << "Max vertex attributes supported: " << maxAttribs << std::endl;
    
    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    
    // Delete any existing buffers
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "Error deleting VAO: 0x" << std::hex << err << std::dec << std::endl;
        }
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "Error deleting VBO: 0x" << std::hex << err << std::dec << std::endl;
        }
    }

    // Clear any existing errors
    while (glGetError() != GL_NO_ERROR) {}
    
    // Generate and bind VAO first
    glGenVertexArrays(1, &VAO);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error generating VAO: 0x" << std::hex << err << std::dec << std::endl;
        throw std::runtime_error("Failed to generate VAO");
    }
    
    if (VAO == 0) {
        throw std::runtime_error("Failed to generate VAO - invalid ID returned");
    }
    
    std::cout << "Generated VAO ID: " << VAO << std::endl;
    
    glBindVertexArray(VAO);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error binding VAO: 0x" << std::hex << err << std::dec << std::endl;
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
        throw std::runtime_error("Failed to bind VAO");
    }
    
    // Generate and bind VBO
    glGenBuffers(1, &VBO);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error generating VBO: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to generate VBO");
    }
    if (VBO == 0) {
        cleanup();
        throw std::runtime_error("Failed to generate VBO - invalid ID returned");
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error binding VBO: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to bind VBO");
    }
    
    // Upload vertex data
    if (vertices.empty()) {
        cleanup();
        throw std::runtime_error("No vertices to upload");
    }
    
    size_t bufferSize = vertices.size() * sizeof(float);
    std::cout << "Uploading " << vertices.size() << " floats (" << vertices.size()/2 
              << " vertices) to VBO, total size: " << bufferSize << " bytes" << std::endl;
    
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices.data(), GL_STATIC_DRAW);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error uploading vertex data: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to upload vertex data");
    }
    
    // Set up vertex attributes while VAO is bound
    glEnableVertexAttribArray(0);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error enabling vertex attrib array: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to enable vertex attribute array");
    }
    
    // Each vertex has 2 floats (x,y), tightly packed
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error setting vertex attrib pointer: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to set vertex attribute pointer");
    }
    
    // Verify attribute array is enabled
    GLint enabled;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    err = glGetError();
    if (err != GL_NO_ERROR || !enabled) {
        std::cerr << "Vertex attribute array 0 is not enabled after setup" << std::endl;
        cleanup();
        throw std::runtime_error("Vertex attribute array verification failed");
    }
    
    // Unbind VBO but keep VAO bound
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Restore previous VAO binding
    glBindVertexArray(previousVAO);
    
    std::cout << "Buffer initialization completed successfully with " 
              << vertices.size() / 2 << " vertices (" 
              << vertices.size() / 4 << " lines)" << std::endl;
}

void SpacetimeGrid::cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
}

void SpacetimeGrid::setupShaderUniforms(const Shader& shader) {
    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    std::cout << "Previous VAO binding in setupShaderUniforms: " << previousVAO << std::endl;

    // Ensure shader is active first
    shader.use();

    // Get uniform locations
    timeLoc = glGetUniformLocation(shader.ID, "time");
    if (timeLoc == -1) {
        std::cerr << "Warning: 'time' uniform not found in grid shader" << std::endl;
    }

    // Bind our VAO
    glBindVertexArray(VAO);
    std::cout << "Binding grid VAO in setupShaderUniforms: " << VAO << std::endl;

    // Set temporary uniform for validation
    if (timeLoc != -1) {
        glUniform1f(timeLoc, 0.0f);
    }
    
    // Verify VAO binding
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != VAO) {
        std::cerr << "Error: VAO binding mismatch in setupShaderUniforms. Expected: " << VAO << ", Got: " << currentVAO << std::endl;
        glBindVertexArray(VAO);  // Try to rebind
    }
    
    // Validate shader with VAO bound and uniforms set
    GLint validateStatus;
    glValidateProgram(shader.ID);
    glGetProgramiv(shader.ID, GL_VALIDATE_STATUS, &validateStatus);
    if (validateStatus == GL_FALSE) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shader.ID, 512, NULL, infoLog);
        std::cerr << "Shader validation failed: " << infoLog << std::endl;
    } else {
        std::cout << "Shader validation successful" << std::endl;
    }

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in setupShaderUniforms: 0x" << std::hex << err << std::dec << std::endl;
    }

    // Restore previous VAO binding
    glBindVertexArray(previousVAO);
    std::cout << "Restored VAO binding in setupShaderUniforms to: " << previousVAO << std::endl;
}

void SpacetimeGrid::drawGrid(const Shader& shader, float time) {
    if (VAO == 0 || VBO == 0) {
        std::cerr << "Warning: Attempting to draw grid with invalid buffers" << std::endl;
        return;
    }

    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    std::cout << "Previous VAO binding before draw: " << previousVAO << std::endl;

    // Ensure shader is active first
    shader.use();
    
    // Bind our VAO
    glBindVertexArray(VAO);
    std::cout << "Binding grid VAO for drawing: " << VAO << std::endl;
    
    // Set time uniform if location is valid
    if (timeLoc != -1) {
        glUniform1f(timeLoc, time);
    }
    
    // Check for OpenGL errors after setting uniforms
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after setting uniforms: 0x" << std::hex << err << std::dec << std::endl;
    }
    
    // Verify VAO is still bound correctly
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != VAO) {
        std::cerr << "Error: VAO not bound correctly in drawGrid. Expected: " << VAO << ", Got: " << currentVAO << std::endl;
        glBindVertexArray(VAO);  // Rebind if necessary
        
        // Verify the rebind worked
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
        if (currentVAO != VAO) {
            std::cerr << "Error: Failed to rebind VAO. Still got: " << currentVAO << std::endl;
            return;  // Abort the draw if we can't get the correct VAO bound
        }
    }
    
    // Verify attribute array is enabled
    GLint enabled;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    if (!enabled) {
        std::cerr << "Warning: Vertex attribute array 0 is not enabled" << std::endl;
        glEnableVertexAttribArray(0);  // Re-enable if necessary
    }
    
    // Draw the lines
    int numLines = vertices.size() / 4;  // Each line has 4 floats (2 vertices * 2 coordinates)
    glDrawArrays(GL_LINES, 0, numLines * 2);  // 2 vertices per line
    
    // Check for OpenGL errors
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in drawGrid: 0x" << std::hex << err << std::dec 
                  << " (drawing " << numLines << " lines)" << std::endl;
    }
    
    // Restore previous VAO binding
    glBindVertexArray(previousVAO);
    std::cout << "Restored VAO binding after draw to: " << previousVAO << std::endl;
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