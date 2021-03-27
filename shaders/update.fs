#version 330 core

uniform int N;
uniform float G;
uniform float dt;
uniform sampler2D positions;
uniform sampler2D velocities;

layout (location = 0) out vec4 posTexel;
layout (location = 1) out vec4 velTexel;

void main()
{
    vec3 pos = texelFetch(positions,  ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 vel = texelFetch(velocities, ivec2(gl_FragCoord.xy), 0).xyz;
  
    int index = int(gl_FragCoord.x);
    for (int i = 0; i < N; i++) {
        if (i == index)
            continue;
        
        vec3 otherPos = texelFetch(positions, ivec2(i,0), 0).xyz;
        vec3 dPos = otherPos - pos;
        float r = length(dPos);

        vec3 dAccel = G / (r * r * r) * dPos;
        vel += dAccel * dt;
    }
    pos += vel * dt;
  
    posTexel = vec4(pos, 1.0f);
    velTexel = vec4(vel, 1.0f);
}