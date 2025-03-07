#include "Simulation.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
              << " type = " << type 
              << ", severity = " << severity 
              << ", message = " << message << std::endl;
}

Simulation::Simulation() : window(nullptr), grid(nullptr), gridShader(nullptr), bodyShader(nullptr) {
    std::cout << "Starting simulation initialization..." << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }
    std::cout << "GLFW initialized successfully" << std::endl;

    // Set OpenGL version and profile for macOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // Use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);  // Enable retina display support

    // Create window
    window = glfwCreateWindow(800, 800, "Einsteinian Solar System", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    std::cout << "Window created successfully" << std::endl;

    // Make OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    std::cout << "OpenGL context made current" << std::endl;

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }
    std::cout << "GLAD initialized successfully" << std::endl;

    // Verify OpenGL capabilities
    if (!GLAD_GL_VERSION_3_3) {
        std::cerr << "OpenGL 3.3 is not supported" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(-1);
    }

    // Check for debug output support
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    bool hasDebugOutput = extensions && strstr(extensions, "GL_KHR_debug");
    
    if (hasDebugOutput) {
        std::cout << "Debug output supported, enabling..." << std::endl;
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
    } else {
        std::cout << "Debug output not supported, falling back to error checking" << std::endl;
    }

    // Print OpenGL information
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    if (!version || !vendor || !renderer || !glslVersion) {
        std::cerr << "Failed to get OpenGL information" << std::endl;
        cleanup();
        exit(-1);
    }
    
    std::cout << "OpenGL Version: " << version << std::endl;
    std::cout << "OpenGL Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Renderer: " << renderer << std::endl;
    std::cout << "GLSL Version: " << glslVersion << std::endl;

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

    try {
        std::cout << "Starting shader initialization..." << std::endl;
        
        // Initialize shaders with detailed error checking
        try {
            std::cout << "Creating grid shader..." << std::endl;
            gridShader = new Shader("grid_vertex_shader.glsl", "grid_fragment_shader.glsl");
            std::cout << "Grid shader initialized successfully with ID: " << gridShader->ID << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize grid shader: " << e.what() << std::endl;
            throw;
        }
        
        try {
            std::cout << "Creating body shader..." << std::endl;
            bodyShader = new Shader("body_vertex_shader.glsl", "body_fragment_shader.glsl");
            std::cout << "Body shader initialized successfully with ID: " << bodyShader->ID << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize body shader: " << e.what() << std::endl;
            throw;
        }

        // Initialize SpacetimeGrid after OpenGL is set up
        try {
            std::cout << "Creating SpacetimeGrid..." << std::endl;
            grid = new SpacetimeGrid();
            std::cout << "SpacetimeGrid created successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to create SpacetimeGrid: " << e.what() << std::endl;
            throw;
        }

        std::cout << "Starting celestial body creation..." << std::endl;
        
        // Add celestial bodies with adjusted sizes
        try {
            // Sun at center, larger radius
            bodies.emplace_back(0.0f, 0.0f, 0.0f, 0.0f, 1.989e30, 0.2f, 1.0f, 0.9f, 0.0f); // Sun (yellow)
            std::cout << "Sun created successfully" << std::endl;
            
            // Earth at x=0.5, smaller radius
            bodies.emplace_back(0.5f, 0.0f, 0.0f, 0.3f, 5.972e24, 0.1f, 0.2f, 0.5f, 1.0f); // Earth (blue)
            std::cout << "Earth created successfully" << std::endl;

            // Initialize shader uniforms for all bodies
            bodyShader->use();
            for (auto& body : bodies) {
                body.setupShaderUniforms(*bodyShader);
            }
            std::cout << "Body shader uniforms initialized" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Failed to create celestial bodies: " << e.what() << std::endl;
            throw;
        }

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
        
    } catch (const std::exception& e) {
        std::cerr << "Error during initialization: " << e.what() << std::endl;
        cleanup();
        exit(-1);
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
    
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float time = glfwGetTime();

        // Store initial VAO binding
        GLint initialVAO;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &initialVAO);
        std::cout << "\nFrame start, initial VAO binding: " << initialVAO << std::endl;

        // Draw the warped spacetime grid
        gridShader->use();
        gridShader->setFloat("time", time);
        grid->drawGrid(*gridShader, time);

        // Verify VAO state after grid drawing
        GLint postGridVAO;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &postGridVAO);
        std::cout << "VAO binding after grid draw: " << postGridVAO << std::endl;

        // Update and render celestial bodies
        bodyShader->use();
        for (auto& body : bodies) {
            body.updatePosition();
            
            // Get VAO binding before drawing body
            GLint preBodyVAO;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &preBodyVAO);
            std::cout << "VAO binding before drawing body: " << preBodyVAO << std::endl;
            
            body.draw(*bodyShader);  // Draw each body right after updating its position
            
            // Verify VAO state after body drawing
            GLint postBodyVAO;
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &postBodyVAO);
            std::cout << "VAO binding after drawing body: " << postBodyVAO << std::endl;
        }

        // Restore initial VAO binding at end of frame
        glBindVertexArray(initialVAO);
        std::cout << "Restored initial VAO binding: " << initialVAO << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error in run loop: 0x" << std::hex << err << std::dec << std::endl;
        }
    }
    std::cout << "Simulation loop ended" << std::endl;
}