#version 330 core

layout (location = 0) in int index;

out vec4 Color;

uniform mat4 mvp;
uniform sampler2D positions;

void main()
{
    gl_Position = mvp * texelFetch(positions, ivec2(index, 0), 0);
    Color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}