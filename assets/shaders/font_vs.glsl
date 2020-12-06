#version 330 core
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTextureCoord;
out vec2 vTextureCoords;
uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(inPosition, 0.0, 1.0);
    vTextureCoords = inTextureCoord;
}