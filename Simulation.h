#ifndef SIMULATION_H
#define SIMULATION_H

#include "CelestialBody.h"
#include "SpacetimeGrid.h"
#include "Shader.h"
#include <vector>
#include <GLFW/glfw3.h>

class Simulation {
private:
    GLFWwindow* window;
    std::vector<CelestialBody> bodies;
    SpacetimeGrid* grid;  // Changed to pointer
    Shader* gridShader;    // Shader for grid
    Shader* bodyShader;    // Shader for celestial bodies

    void cleanup();        // Helper method to clean up resources

public:
    Simulation();          // Constructor
    ~Simulation();         // Destructor
    void run();           // Runs the simulation loop
};

#endif // SIMULATION_H