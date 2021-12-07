#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inLightPos;

layout(location = 0) out float outColor;


void main() 
{
    vec3 lightVec = inPos.xyz - inLightPos;
    outColor = length(lightVec)+0.5;
}