#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Transforms {
    mat4 model;
    mat4 modelView;
    mat4 modelViewProj;
} PushConstants;

layout(set = 0, binding = 0) uniform mvScene
{

    bool doLighting;
    bool doNormal;
    bool doSpecular;
    bool doDiffuse;
    //-------------------------- ( 16 bytes )

    bool doOmniShadows;
    bool doDirectionalShadows;
    bool doSkybox;
    bool doPCF;
    //-------------------------- ( 16 bytes )
    
    int pcfRange;
    //-------------------------- ( 16 bytes )

    mat4 directionalShadowView;
    //-------------------------- ( 64 bytes )

    mat4 directionalShadowProjection;
    //-------------------------- ( 64 bytes )

    //-------------------------- ( 1*16 + 2*64= 144 bytes )
} scene;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 viewPos;
layout(location = 1) out vec3 viewNormal;
layout(location = 2) out vec3 worldNormal;
layout(location = 3) out vec2 texCoord;
layout(location = 4) out vec4 dshadowWorldPos;
layout(location = 5) out mat3 tangentBasis;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, -0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


vec4 ToDirectShadowHomoSpace(vec3 pos, mat4 modelTransform)
{
    return (biasMat * scene.directionalShadowProjection * scene.directionalShadowView * modelTransform) * vec4(pos, 1.0);
}

void main() 
{
    gl_Position = PushConstants.modelViewProj * vec4(inPosition, 1.0);
    viewPos = vec3(PushConstants.modelView * vec4(inPosition, 1.0));
    viewNormal = mat3(PushConstants.modelView) * inNormal;
    worldNormal = mat3(PushConstants.model) * inNormal;
    texCoord = inTexCoord;
    vec3 T = mat3(PushConstants.modelView)*inTangent;
    vec3 B = mat3(PushConstants.modelView)*inBitangent;
    vec3 N = mat3(PushConstants.modelView)*inNormal;
    tangentBasis = mat3(T, B, N);
    dshadowWorldPos = ToDirectShadowHomoSpace(inPosition, PushConstants.model);
}