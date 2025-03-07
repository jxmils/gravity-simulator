#ifndef SIMULATION_H
#define SIMULATION_H

#include "CelestialBody.h"
#include "SpacetimeGrid.h"
#include "Shader.h"
#include <vector>
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
    float zoom;           // Zoom level
    
    // Camera parameters
    float rotationX;  // Rotation around X-axis (pitch)
    float rotationY;  // Rotation around Y-axis (yaw)
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    void cleanup();        // Helper method to clean up resources
    void updateCameraMatrices();  // New method to update view/projection matrices
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static Simulation* instance;  // Singleton instance for callbacks

public:
    Simulation();          // Constructor
    ~Simulation();         // Destructor
    void run();           // Runs the simulation loop
    void handleKeyPress(int key, int action);  // Handles keyboard input
};

#endif // SIMULATION_H