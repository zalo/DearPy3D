#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Transforms {
    mat4 modelViewProj;
    vec4 lightPos;
} PushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec3 outLightPos;

void main() 
{
    gl_Position = PushConstants.modelViewProj * vec4(inPosition, 1.0);
	outPos = vec4(inPosition, 1.0);	
	outLightPos = PushConstants.lightPos.xyz; 
}