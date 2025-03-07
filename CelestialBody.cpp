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
    cleanup();
}

void CelestialBody::initializeBuffers() {
    // Generate circle vertices
    const int segments = 32;  // Reduced for better performance, still smooth
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 2);  // Reserve space for all vertices
    
    // Center vertex
    vertices.push_back(0.0f);  // x
    vertices.push_back(0.0f);  // y
    
    // Generate circle vertices
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        vertices.push_back(cosf(theta));  // x
        vertices.push_back(sinf(theta));  // y
    }

    // Store vertex count for drawing
    vertexCount = vertices.size() / 2;  // Number of vertices (not floats)
    std::cout << "Initializing CelestialBody buffers with " << vertexCount << " vertices" << std::endl;

    // Clear any existing errors
    while (glGetError() != GL_NO_ERROR) {}

    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    std::cout << "Previous VAO binding before initialization: " << previousVAO << std::endl;

    // Delete any existing buffers
    cleanup();

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
    std::cout << "Generated CelestialBody VAO ID: " << VAO << std::endl;

    glBindVertexArray(VAO);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "Error binding VAO: 0x" << std::hex << err << std::dec << std::endl;
        cleanup();
        throw std::runtime_error("Failed to bind VAO");
    }
    std::cout << "Binding CelestialBody VAO: " << VAO << std::endl;

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
    size_t bufferSize = vertices.size() * sizeof(float);
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
    if (!enabled) {
        std::cerr << "Warning: Vertex attribute array 0 is not enabled" << std::endl;
        glEnableVertexAttribArray(0);
    }

    // Unbind VBO but keep VAO bound
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Keep VAO bound and verify final state
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != VAO) {
        std::cerr << "Error: VAO binding lost at end of initialization. Expected: " << VAO << ", Got: " << currentVAO << std::endl;
        cleanup();
        throw std::runtime_error("VAO binding verification failed");
    }

    // Keep our VAO bound instead of restoring previous
    std::cout << "Keeping VAO " << VAO << " bound at end of initialization" << std::endl;

    std::cout << "Buffer initialization completed successfully with " 
              << vertices.size() / 2 << " vertices" << std::endl;
}

void CelestialBody::setupShaderUniforms(const Shader& shader) {
    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    std::cout << "Previous VAO binding in setupShaderUniforms: " << previousVAO << std::endl;

    // Ensure shader is active first
    shader.use();

    // Get uniform locations
    modelLoc = glGetUniformLocation(shader.ID, "model");
    colorLoc = glGetUniformLocation(shader.ID, "color");
    
    if (modelLoc == -1) {
        std::cerr << "Warning: 'model' uniform not found in shader" << std::endl;
    }
    if (colorLoc == -1) {
        std::cerr << "Warning: 'color' uniform not found in shader" << std::endl;
    }

    // Now bind our VAO
    glBindVertexArray(VAO);
    std::cout << "Binding VAO in setupShaderUniforms: " << VAO << std::endl;

    // Verify VAO binding
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != VAO) {
        std::cerr << "Error: VAO binding mismatch in setupShaderUniforms. Expected: " << VAO << ", Got: " << currentVAO << std::endl;
        // Attempt to rebind
        glBindVertexArray(VAO);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
        if (currentVAO != VAO) {
            // Try reinitializing buffers as a last resort
            std::cout << "Attempting to recover by reinitializing buffers..." << std::endl;
            initializeBuffers();
            glBindVertexArray(VAO);
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
            if (currentVAO != VAO) {
                throw std::runtime_error("VAO binding verification failed after recovery attempts");
            }
        }
    }

    // Create a temporary identity matrix for validation
    float identityMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Set temporary uniforms for validation
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, identityMatrix);
    }
    if (colorLoc != -1) {
        glUniform3fv(colorLoc, 1, color);
    }

    // Verify attribute array is enabled
    GLint enabled;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    if (!enabled) {
        std::cerr << "Warning: Vertex attribute array 0 is not enabled" << std::endl;
        glEnableVertexAttribArray(0);
    }

    // Validate shader with VAO bound and uniforms set
    GLint validateStatus;
    glValidateProgram(shader.ID);
    glGetProgramiv(shader.ID, GL_VALIDATE_STATUS, &validateStatus);
    if (validateStatus == GL_FALSE) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shader.ID, 512, NULL, infoLog);
        std::cerr << "Shader validation failed: " << infoLog << std::endl;
    }

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in setupShaderUniforms: 0x" << std::hex << err << std::dec << std::endl;
    }

    // Keep our VAO bound instead of restoring previous
    std::cout << "Keeping VAO " << VAO << " bound at end of setupShaderUniforms" << std::endl;
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
    if (VAO == 0 || VBO == 0) {
        std::cerr << "Warning: Attempting to draw with invalid buffers" << std::endl;
        return;
    }

    // Store current VAO binding to restore later
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    std::cout << "Previous VAO binding before celestial body draw: " << previousVAO << std::endl;

    // Ensure shader is active first
    shader.use();
    
    // Set up uniforms if not already done
    if (modelLoc == -1 || colorLoc == -1) {
        setupShaderUniforms(shader);
    }

    // Create model matrix for position and scale (column-major order)
    float modelMatrix[16] = {
        radius, 0.0f, 0.0f, 0.0f,   // First column (scale x)
        0.0f, radius, 0.0f, 0.0f,   // Second column (scale y)
        0.0f, 0.0f, 1.0f, 0.0f,     // Third column
        x, y, 0.0f, 1.0f            // Fourth column (translation)
    };

    // Bind our VAO
    std::cout << "Drawing CelestialBody, binding VAO: " << VAO << std::endl;
    glBindVertexArray(VAO);

    // Verify VAO binding
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
    if (currentVAO != VAO) {
        std::cerr << "Error: VAO not bound correctly in CelestialBody draw. Expected: " << VAO << ", Got: " << currentVAO << std::endl;
        
        // Try to recover by reinitializing buffers
        std::cout << "Attempting to recover by reinitializing buffers..." << std::endl;
        initializeBuffers();
        glBindVertexArray(VAO);
        
        // Verify VAO again after recovery
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
        if (currentVAO != VAO) {
            std::cerr << "Error: Failed to recover VAO binding. Got: " << currentVAO << std::endl;
            return;
        }
    }

    // Set uniforms with VAO bound
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);
    }
    if (colorLoc != -1) {
        glUniform3fv(colorLoc, 1, color);
    }

    // Verify attribute array is enabled
    GLint enabled;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    if (!enabled) {
        std::cerr << "Warning: Vertex attribute array 0 is not enabled" << std::endl;
        glEnableVertexAttribArray(0);
    }

    // Draw the circle
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
    
    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in CelestialBody draw: 0x" << std::hex << err << std::dec 
                  << " (drawing circle with " << vertexCount << " vertices)" << std::endl;
    }
    
    // Restore previous VAO binding only if it was different
    if (previousVAO != VAO) {
        glBindVertexArray(previousVAO);
        std::cout << "Restored previous VAO binding: " << previousVAO << std::endl;
    } else {
        std::cout << "Keeping current VAO binding: " << VAO << std::endl;
    }
}

void CelestialBody::cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
}