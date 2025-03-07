#version 330 core
layout (location = 0) in vec2 aPos;
uniform float time;
uniform mat4 view;
uniform mat4 projection;
uniform float zoom = 1.0;

void main() {
    // Start with 2D position
    vec3 pos = vec3(aPos.x, aPos.y, 0.0);
    
    // Apply gravitational warping effect
    if (time > 0.0) {
        // Calculate distance from center (0,0)
        float dist = length(aPos);
        // Create more pronounced warping effect
        float warpFactor = 0.1 / (1.0 + dist * 2.0);
        pos.z = -warpFactor * 2.0; // More visible warping in Z direction
        
        // Add wave effect
        pos.z += sin(pos.x * 5.0 + time) * 0.02;
    }
    
    // Apply zoom
    pos *= zoom;
    
    // Apply view and projection transformations
    gl_Position = projection * view * vec4(pos, 1.0);
} 