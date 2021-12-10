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

    mat4 pointShadowView;
    //-------------------------- ( 64 bytes )

    mat4 directionalShadowView;
    //-------------------------- ( 64 bytes )

    mat4 directionalShadowProjection;
    //-------------------------- ( 64 bytes )

    //-------------------------- ( 1*16 + 3*64= 144 bytes )
} scene;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 n;
layout(location = 2) in vec3 t;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 tc;

layout(location = 0) out vec3 Pos;
layout(location = 1) out vec3 WorldPos;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec2 UV;
layout(location = 4) out vec3 Tangent;
layout(location = 5) out vec4 dshadowWorldPos;
layout(location = 6) out vec4 oshadowWorldPos;
layout(location = 7) out vec3 ViewPos;
layout(location = 8) out vec3 ViewNormal;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, -0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

vec4 ToShadowHomoSpace(vec3 pos, mat4 modelTransform)
{
    vec4 world = modelTransform * vec4(pos, 1.0);
    return scene.pointShadowView * world;
}

vec4 ToDirectShadowHomoSpace(vec3 pos, mat4 modelTransform)
{
    return (biasMat * scene.directionalShadowProjection * scene.directionalShadowView * modelTransform) * vec4(pos, 1.0);
}

void main() 
{
    gl_Position = PushConstants.modelViewProj * vec4(pos, 1.0);
    vec4 locPos = PushConstants.model * vec4(pos, 1.0);
    ViewPos = vec3(PushConstants.modelView * vec4(pos, 1.0));
    ViewNormal = mat3(PushConstants.modelView) * n;
    WorldPos = locPos.xyz;
    Pos = gl_Position.xyz;
    Normal = mat3(PushConstants.model) * n;
    Tangent = mat3(PushConstants.model) * t;
    UV = tc;
    dshadowWorldPos = ToDirectShadowHomoSpace(pos, PushConstants.model);
    oshadowWorldPos = ToShadowHomoSpace(pos, PushConstants.model);
}