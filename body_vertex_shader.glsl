#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    // Calculate normal (for a sphere, normal is the same as position for unit sphere)
    Normal = normalize(mat3(model) * aPos);
    
    // Calculate fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Final position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
} 