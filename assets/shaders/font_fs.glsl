#version 330 core
precision mediump float;
in vec2 vTextureCoords;
out vec4 outColor;

uniform sampler2D sTexture;
uniform vec3 textColor;

void main () {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(sTexture, vTextureCoords).r);
    outColor = vec4(textColor, 1.0) * sampled;
}