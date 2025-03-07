#version 330 core
layout (location = 0) in vec2 aPos;
uniform float time;  // Time uniform for animation

void main() {
    // Basic position without modification first
    vec2 pos = aPos;
    
    // Only apply time-based effect if time is available
    // This prevents undefined behavior if uniform is not set
    if (time > 0.0) {
        pos.y += sin(pos.x * 5.0 + time) * 0.02;
    }
    
    gl_Position = vec4(pos, 0.0, 1.0);
} 