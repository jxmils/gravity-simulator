#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float zoom = 1.0;

void main() {
    // Convert 2D position to 3D
    vec4 pos = model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    
    // Apply view and projection transformations
    gl_Position = projection * view * vec4(pos.xyz * zoom, 1.0);
} 