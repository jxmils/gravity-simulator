#include "Simulation.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Simulation* Simulation::instance = nullptr;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
              << " type = " << type 
              << ", severity = " << severity 
              << ", message = " << message << std::endl;
}

Simulation::Simulation() : window(nullptr), gridShader(nullptr), bodyShader(nullptr), textShader(nullptr),
                         grid(nullptr), timeAcceleration(1.0f), zoom(1.0f), rotation(0.0f),
                         maxTimeAcceleration(100.0f) {
    instance = this;  // Set singleton instance
    std::cout << "Starting simulation initialization..." << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    std::cout << "GLFW initialized successfully" << std::endl;

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);  // Enable retina display support

    // Create window
    window = glfwCreateWindow(1600, 1600, "Gravity Simulator", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    std::cout << "Window created successfully" << std::endl;

    // Make OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    std::cout << "OpenGL context made current" << std::endl;

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    std::cout << "GLAD initialized successfully" << std::endl;

    // Verify OpenGL capabilities
    if (!GLAD_GL_VERSION_3_3) {
        std::cerr << "OpenGL 3.3 is not supported" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }

    // Set up debug output if available
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::cout << "Debug output enabled" << std::endl;
    } else {
        std::cout << "Debug output not supported, falling back to error checking" << std::endl;
    }

    // Print OpenGL information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Get framebuffer size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::cout << "Framebuffer size: " << width << "x" << height << std::endl;

    // Set viewport and enable depth testing
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);  // Dark blue background

    // Initialize shaders
    std::cout << "Starting shader initialization..." << std::endl;
    try {
        // Create grid shader
        std::cout << "Creating grid shader..." << std::endl;
        gridShader = new Shader("grid_vertex_shader.glsl", "grid_fragment_shader.glsl");
        std::cout << "Grid shader initialized successfully with ID: " << gridShader->ID << std::endl;
        
        // Create body shader
        std::cout << "Creating body shader..." << std::endl;
        bodyShader = new Shader("body_vertex_shader.glsl", "body_fragment_shader.glsl");
        std::cout << "Body shader initialized successfully with ID: " << bodyShader->ID << std::endl;
        
        // Create text shader
        std::cout << "Creating text shader..." << std::endl;
        textShader = new Shader("text_vertex_shader.glsl", "text_fragment_shader.glsl");
        if (!textShader->ID) {
            throw std::runtime_error("Failed to create text shader");
        }
        std::cout << "Text shader initialized successfully with ID: " << textShader->ID << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize text shader: " << e.what() << std::endl;
        cleanup();
        throw;
    }

    // Set up camera
    updateCameraMatrices();
    
    // Store instance for callbacks
    instance = this;
    
    // Set up keyboard callback
    glfwSetKeyCallback(window, keyCallback);

    // Create celestial bodies
    // Sun at center with mass 1.0 (normalized units)
    bodies.emplace_back(
        0.0f, 0.0f,                // x, y position
        0.0f, 0.0f,                // vx, vy velocity
        1.0f,                      // mass
        0.5f,                      // radius (increased from 0.2f)
        1.0f, 0.9f, 0.0f          // r, g, b color
    );
    
    // Earth with elliptical orbit
    // Semi-major axis = 1.0 AU (normalized)
    // Eccentricity = 0.0167 (Earth's actual eccentricity)
    bodies.emplace_back(
        1.0f, 0.0f,                // x, y position (start at (1,0))
        0.0f, 1.0f,                // vx, vy velocity (circular orbit)
        0.000003f,                 // mass (relative to Sun)
        0.15f,                     // visible radius
        0.0f, 0.7f, 1.0f          // r, g, b color
    );
    
    // Create grid
    grid = new SpacetimeGrid();

    // Register window resize callback
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        glViewport(0, 0, width, height);
    });
    std::cout << "Window resize callback registered" << std::endl;

    // Check for OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error during initialization: 0x" << std::hex << err << std::dec << std::endl;
    }
    
    std::cout << "Simulation initialization completed successfully" << std::endl;
}

void Simulation::cleanup() {
    std::cout << "Starting cleanup..." << std::endl;
    
    if (grid) {
        delete grid;
        grid = nullptr;
        std::cout << "SpacetimeGrid cleaned up" << std::endl;
    }
    
    if (gridShader) {
        delete gridShader;
        gridShader = nullptr;
        std::cout << "Grid shader cleaned up" << std::endl;
    }
    
    if (bodyShader) {
        delete bodyShader;
        bodyShader = nullptr;
        std::cout << "Body shader cleaned up" << std::endl;
    }
    
    if (textShader) {
        delete textShader;
        textShader = nullptr;
        std::cout << "Text shader cleaned up" << std::endl;
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
        std::cout << "Window destroyed" << std::endl;
    }
    
    glfwTerminate();
    std::cout << "GLFW terminated" << std::endl;
}

Simulation::~Simulation() {
    cleanup();
}

void Simulation::run() {
    std::cout << "Starting simulation loop..." << std::endl;
    float lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = (currentTime - lastTime) * timeAcceleration;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera matrices
        updateCameraMatrices();

        // Draw the warped spacetime grid
        gridShader->use();
        gridShader->setFloat("time", currentTime);
        gridShader->setMat4("view", viewMatrix);
        gridShader->setMat4("projection", projectionMatrix);
        gridShader->setFloat("zoom", zoom);
        gridShader->setFloat("rotation", rotation);
        
        grid->drawGrid(*gridShader, currentTime);

        // Update and render celestial bodies with accelerated time
        bodyShader->use();
        bodyShader->setMat4("view", viewMatrix);
        bodyShader->setMat4("projection", projectionMatrix);
        
        // Update Earth's position using Kepler's equations
        float simulationTime = currentTime * timeAcceleration;
        for (size_t i = 1; i < bodies.size(); i++) {  // Skip Sun (index 0)
            bodies[i].updatePosition(deltaTime);
        }

        // Draw all celestial bodies
        for (const auto& body : bodies) {
            body.draw(*bodyShader);
        }

        // Draw time acceleration text
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        // Create text projection matrix for screen space
        glm::mat4 textProjection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
        
        textShader->use();
        textShader->setMat4("projection", textProjection);
        
        // Draw text in top-right corner
        float quadWidth = 200.0f;
        float quadHeight = 50.0f;
        float x = width - quadWidth - 10.0f;
        float y = height - quadHeight - 10.0f;
        
        // Draw text background
        glUniform4f(glGetUniformLocation(textShader->ID, "color"), 0.0f, 0.0f, 0.0f, 0.3f);
        drawTextBackground(x, y, quadWidth, quadHeight);
        
        // Draw text
        glUniform4f(glGetUniformLocation(textShader->ID, "color"), 1.0f, 1.0f, 1.0f, 1.0f);
        std::string text = "Time: " + std::to_string((int)timeAcceleration) + "x";
        drawText(text, x + 10.0f, y + 10.0f, 0.5f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    std::cout << "\nSimulation loop ended" << std::endl;
}

void Simulation::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (instance == nullptr) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_LEFT:
                instance->rotation -= 0.1f;
                break;
            case GLFW_KEY_RIGHT:
                instance->rotation += 0.1f;
                break;
            case GLFW_KEY_EQUAL:  // '=' key for zoom in
                instance->zoom *= 1.1f;
                break;
            case GLFW_KEY_MINUS:  // '-' key for zoom out
                instance->zoom /= 1.1f;
                break;
            case GLFW_KEY_LEFT_BRACKET:  // '[' key to decrease time speed
                instance->timeAcceleration = std::max(1.0f, instance->timeAcceleration / 2.0f);
                break;
            case GLFW_KEY_RIGHT_BRACKET:  // ']' key to increase time speed
                instance->timeAcceleration = std::min(instance->maxTimeAcceleration, instance->timeAcceleration * 2.0f);
                break;
        }
    }
}

void Simulation::updateCameraMatrices() {
    // Start with identity matrices
    viewMatrix = glm::mat4(1.0f);
    
    // Set up orthographic projection with zoom
    float orthoSize = 2.0f / zoom;  // Smaller view size
    projectionMatrix = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, -10.0f, 10.0f);
    
    // Apply camera transformations in correct order
    viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -2.0f));  // Move camera closer
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotation), glm::vec3(1.0f, 0.0f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(panX, panY, 0.0f));
}

void Simulation::drawTextBackground(float x, float y, float width, float height) {
    // Create vertices for a quad
    float vertices[] = {
        x, y, 0.0f, 0.0f, 0.0f,
        x + width, y, 0.0f, 1.0f, 0.0f,
        x, y + height, 0.0f, 0.0f, 1.0f,
        x + width, y + height, 0.0f, 1.0f, 1.0f
    };
    
    // Create and bind VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Create and bind VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Simulation::drawText(const std::string& text, float x, float y, float scale) {
    // For now, we'll just draw a simple quad as placeholder text
    // In a real implementation, you would use a font texture atlas and render actual text
    float width = text.length() * 20.0f * scale;  // Approximate width based on text length
    float height = 30.0f * scale;  // Fixed height
    
    // Create vertices for a quad
    float vertices[] = {
        x, y, 0.0f, 0.0f, 0.0f,
        x + width, y, 0.0f, 1.0f, 0.0f,
        x, y + height, 0.0f, 0.0f, 1.0f,
        x + width, y + height, 0.0f, 1.0f, 1.0f
    };
    
    // Create and bind VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Create and bind VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}