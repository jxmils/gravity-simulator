#version 330 core
layout (location = 0) in vec2 aPos;

uniform float time;
uniform mat4 view;
uniform mat4 projection;
uniform float zoom = 1.0;
uniform float rotation = 0.0;

void main() {
    // Scale grid to match astronomical units (1 AU = Earth-Sun distance)
    vec3 pos = vec3(aPos.x * 2.0, aPos.y * 2.0, 0.0);  // Smaller grid scale for visibility
    
    // Apply rotation
    float cosRot = cos(rotation);
    float sinRot = sin(rotation);
    vec3 rotatedPos = vec3(
        pos.x * cosRot - pos.y * sinRot,
        pos.x * sinRot + pos.y * cosRot,
        pos.z
    );
    
    // Apply zoom
    rotatedPos *= zoom;
    
    // Calculate distance from center (Sun's position)
    float dist = length(rotatedPos);
    
    // Constants for gravitational warping
    float G = 6.67430e-11;  // Gravitational constant
    float M = 1.989e30;     // Solar mass in kg
    float scale = 1.0e-9;   // Scale factor to make warping visible
    float minDist = 0.1;    // Minimum distance to prevent extreme warping
    
    // Calculate gravitational potential (φ = -GM/r)
    float potential = G * M / (max(dist, minDist));
    
    // Add time-based oscillation to the warping
    float timeScale = 0.5;  // Controls the speed of oscillation
    float oscillation = sin(time * timeScale) * 0.1;  // Small oscillation factor
    
    // Apply warping based on gravitational potential with time-based oscillation
    rotatedPos.z = -potential * scale * (1.0 + oscillation);
    
    // Apply view and projection transformations
    gl_Position = projection * view * vec4(rotatedPos, 1.0);
} 