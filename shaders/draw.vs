#version 330 core

layout (location = 0) in int index;

out vec4 Color;

uniform mat4 mvp;
uniform sampler2D positions;
uniform sampler2D velocities;

void main()
{
    vec3 pos = texelFetch(positions,  ivec2(index, 0), 0).xyz;
    vec3 vel = texelFetch(velocities, ivec2(index, 0), 0).xyz;
    // float r = length(vel) / 100;
    // float g = 1 - r;
    // float b = 0;
    float r = -pos.x;
    float g = pos.x;
    float b = 1.0;

    gl_Position = mvp * vec4(pos, 1.0f);
    Color = vec4(r, g, b, 1);
}