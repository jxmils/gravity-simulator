#ifndef SIMULATION_H
#define SIMULATION_H

#include "CelestialBody.h"
#include "SpacetimeGrid.h"
#include "Shader.h"
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Simulation {
private:
    GLFWwindow* window;
    std::vector<CelestialBody> bodies;
    SpacetimeGrid* grid;  // Changed to pointer
    Shader* gridShader;    // Shader for grid
    Shader* bodyShader;    // Shader for celestial bodies
    Shader* textShader;    // Shader for text rendering
    float zoom;           // Zoom level
    float rotation;       // Rotation angle
    
    // Camera parameters
    float rotationX;  // Rotation around X-axis (pitch)
    float rotationY;  // Rotation around Y-axis (yaw)
    float panX;  // Add panning variables
    float panY;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    // Time control
    float timeAcceleration;  // Current time acceleration factor
    const float maxTimeAcceleration;  // Maximum allowed time acceleration
    
    void cleanup();        // Helper method to clean up resources
    void updateCameraMatrices();  // New method to update view/projection matrices
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static Simulation* instance;  // Singleton instance for callbacks
    
    // Text rendering functions
    void drawTextBackground(float x, float y, float width, float height);
    void drawText(const std::string& text, float x, float y, float scale);

public:
    Simulation();          // Constructor
    ~Simulation();         // Destructor
    void run();           // Runs the simulation loop
    void handleKeyPress(int key, int action);  // Handles keyboard input
};

#endif // SIMULATION_H