#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;

layout(push_constant) uniform Transforms {
    mat4 modelViewProj;
} PushConstants;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec4 pos;

void main() 
{
	worldPos = inPos;
	pos = PushConstants.modelViewProj * vec4(inPos.xyz, 0.0);
	pos.z = pos.w;
	gl_Position = pos;

}
