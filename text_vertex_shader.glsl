#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform vec4 color;

out vec4 TextColor;

void main() {
    // Convert to clip space coordinates
    vec4 clipPos = projection * vec4(aPos.xy, 0.0, 1.0);
    gl_Position = clipPos;
    TextColor = color;
} 