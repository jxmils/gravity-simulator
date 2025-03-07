# Gravity Simulator

This project is an advanced OpenGL-based simulation designed to provide deep insights into gravitational phenomena. The primary objective is to create a visual and interactive representation of gravitational interactions within a simplified solar system model. The simulator leverages modern graphics programming techniques (VAOs, VBOs, and shader-based rendering) to render celestial bodies and a dynamically deformed spacetime grid.

## Motivation

Understanding gravity, particularly in the context of general relativity, remains one of the fundamental challenges in theoretical physics and astrophysics. This simulation serves as a conceptual and pedagogical tool aimed at elucidating how mass distorts spacetime and influences the motion of nearby objects. By providing an interactive environment, the project seeks to:
- Enhance the intuitive comprehension of gravitational effects.
- Explore the nuances of gravitational potential and orbital dynamics.
- Serve as a resource for graduate-level research and instruction in gravitational physics.

## Technical Overview

The simulator is implemented using modern OpenGL and C++17. It makes extensive use of the following libraries:
- **GLFW**: For window creation and input handling.
- **GLAD**: For OpenGL function loading.
- **GLM**: For mathematical computations, including matrix transformations.
- **CMake**: For building the project in a cross-platform manner.

### Key Components

- **Shader Programs**:  
  Custom GLSL shaders are utilized for both the grid and celestial bodies. The grid shader incorporates a time uniform to animate spacetime deformations, while the body shader manages model transformations and color assignments. Robust shader management techniques (including uniform caching and validation) ensure that rendering is both efficient and reliable.

- **Spacetime Grid**:  
  A dynamically generated grid represents the curvature of spacetime. The grid is deformed in real-time based on a simplified model of gravitational distortion, providing a visual representation of the gravitational potential produced by massive objects.

- **Celestial Bodies**:  
  Objects such as the Sun and Earth are rendered using modern OpenGL practices (VAOs, VBOs) to ensure robust rendering and resource management. Their motion is computed using fundamental orbital dynamics, with potential extensions to include relativistic corrections.

- **Simulation Loop**:  
  The main simulation loop integrates real-time rendering with physics updates, supporting interactive exploration of gravitational effects.

- **Interactive Controls**:  
  To facilitate an in-depth exploration of gravitational phenomena, several interactive controls have been implemented:
  - **Zoom Controls**: Use the `-` key to zoom out and the `=` key to zoom in, allowing examination of both large-scale spacetime curvature and fine orbital details.
  - **Rotation Controls**: The arrow keys enable rotation of the view, providing diverse perspectives on the gravitational field.
  - **Time-Speed Controls**: The `[` and `]` keys decrease and increase the simulation speed, respectively, permitting the user to observe both rapid and gradual dynamical changes.

## Research Implications

This simulation is intended as a conceptual framework to enhance the understanding of gravity from both classical and relativistic perspectives. While the current model employs a simplified representation of gravitational interactions, it establishes a basis for further, more sophisticated explorations of spacetime curvature and orbital mechanics. Future work may include:
- Incorporating relativistic corrections based on the Schwarzschild metric.
- Expanding the simulation to include additional celestial bodies and more complex gravitational interactions.
- Investigating phenomena such as gravitational lensing and frame-dragging.

### Building the Project
```sh
brew install glfw glm
```
### Clone the repository and create a build directory:
```sh
git clone <repository_url>
cd gravity-simulator
```

### Run the `run.sh` script:
```sh
./run.sh
```
