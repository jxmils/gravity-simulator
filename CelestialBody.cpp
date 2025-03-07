#include "CelestialBody.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Constants
const float G = 6.67430e-11;  // Gravitational constant
const float c = 3e8;          // Speed of light
const float dt = 0.0001f;     // Smaller timestep
const float SCALE = 1e-9f;    // Scale factor to bring astronomical units into view

CelestialBody::CelestialBody(float x, float y, float vx, float vy, float mass, float radius, float r, float g, float b)
    : x(x), y(y), vx(vx), vy(vy), mass(mass), radius(radius), VAO(0), VBO(0), modelLoc(-1), colorLoc(-1) {
    color[0] = r; color[1] = g; color[2] = b;
    initializeBuffers();
}

CelestialBody::~CelestialBody() {
    cleanup();
}

void CelestialBody::initializeBuffers() const {
    // Generate vertices for a sphere using triangles
    std::vector<float> vertices;
    const int latitudeBands = 30;
    const int longitudeBands = 30;
    const float PI = 3.14159265359f;
    
    // Generate sphere vertices
    for (int lat = 0; lat <= latitudeBands; lat++) {
        float theta = lat * PI / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= longitudeBands; lon++) {
            float phi = lon * 2 * PI / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta * 0.5f;  // radius = 0.5 to match previous size
            float y = cosTheta * 0.5f;
            float z = sinPhi * sinTheta * 0.5f;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate indices for triangle strips
    std::vector<unsigned int> indices;
    for (int lat = 0; lat < latitudeBands; lat++) {
        for (int lon = 0; lon < longitudeBands; lon++) {
            int first = (lat * (longitudeBands + 1)) + lon;
            int second = first + longitudeBands + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    vertexCount = indices.size();
    std::cout << "Initializing CelestialBody buffers with " << vertexCount << " vertices for sphere" << std::endl;

    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind VBO for vertices
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Generate and bind EBO for indices
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Print debug info
    std::cout << "Generated sphere with " << vertices.size()/3 << " vertices and " << indices.size() << " indices" << std::endl;
}

void CelestialBody::setupShaderUniforms(const Shader& shader) const {
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
    if (r < 1e-10) {  // Prevent division by zero
        ax = ay = 0.0f;
        return;
    }
    float r3 = r * r * r;
    float factor = -G * mass / r3;
    float relativistic_correction = 1.0f + (3.0f * G * mass) / (c * c * r);
    ax = factor * x * relativistic_correction;
    ay = factor * y * relativistic_correction;
}

void CelestialBody::updatePosition(float deltaTime) {
    float ax = 0.0f, ay = 0.0f;
    computeAcceleration(ax, ay);
    
    // Update velocity using acceleration (a = F/m)
    vx += ax * deltaTime;
    vy += ay * deltaTime;
    
    // Update position using velocity
    x += vx * deltaTime;
    y += vy * deltaTime;
}

void CelestialBody::draw(const Shader& shader) const {
    // Scale positions to visible range
    float scaled_x = x * SCALE;
    float scaled_y = y * SCALE;

    // Create model matrix using GLM
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(scaled_x, scaled_y, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(radius * 0.5f, radius * 0.5f, radius * 0.5f));

    // Bind shader first
    shader.use();
    
    // Set up uniforms if not already done
    if (modelLoc == -1 || colorLoc == -1) {
        setupShaderUniforms(shader);
    }

    // Set uniforms
    if (modelLoc != -1) {
        shader.setMat4("model", modelMatrix);
    }
    if (colorLoc != -1) {
        shader.setVec3("color", color[0], color[1], color[2]);
    }

    // Bind VAO and draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
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