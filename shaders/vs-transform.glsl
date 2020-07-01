#version 460

layout (location = 0) in vec2 in_pos;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(in_pos, 0.0, 1.0);
}
