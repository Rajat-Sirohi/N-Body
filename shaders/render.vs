#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vel;

out vec4 Color;

uniform mat4 mvp;

void main()
{
   gl_Position = mvp * vec4(pos, 1.0f);
   Color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
};