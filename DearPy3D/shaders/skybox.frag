#version 450

layout (set = 0, binding = 0) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec4 pos;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 worldPosMod = worldPos;
	worldPosMod.z = -worldPos.z;
	outFragColor = texture(samplerCubeMap, worldPosMod);
}