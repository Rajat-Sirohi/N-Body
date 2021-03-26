#version 330 core

in vec3 pos;

out vec4 Color;

uniform mat4 mvp;

void main()
{
   gl_Position = mvp * vec4(pos, 1.0f);
   // Color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
   Color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
};