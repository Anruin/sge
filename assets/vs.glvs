#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 intex1;
layout(location = 3) in vec3 intex4;

// Output data ; will be interpolated for each fragment.
out vec3 norm;
out vec3 pos;
out vec3 eyepos;
out vec4 tex1;
out vec3 tex4;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 M;
uniform vec3 eyePosition;

void main(){

norm = (M*vec4(normal,0)).xyz;
pos = (M*vec4(position,1)).xyz;
eyepos = eyePosition;
tex1 = intex1;
tex4 = intex4;

// Output position of the vertex, in clip space : MVP * position
gl_Position =  MVP * vec4(position,1);

}