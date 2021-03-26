#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inVel;

out vec3 outPos;
out vec3 outVel;

uniform float dt;

void main()
{
   outVel = inVel;
   outPos = inPos + outVel*dt;
};