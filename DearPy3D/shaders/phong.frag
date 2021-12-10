#version 450
#extension GL_ARB_separate_shader_objects : enable

#define EPSILON 0.01
#define SHADOW_OPACITY 0.0

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 WorldPos;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec2 UV;
layout(location = 4) in vec3 Tangent;
layout(location = 5) in vec4 dshadowWorldPos;
layout(location = 6) in vec4 oshadowWorldPos;
layout(location = 7) in vec3 ViewPos;
layout(location = 8) in vec3 ViewNormal;


layout(location = 0) out vec4 outColor;

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

    vec3 camPos;    
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

layout(set = 0, binding = 1) uniform mvPointLight 
{
    vec3 viewLightPos;
    //-------------------------- ( 16 bytes )
    
    vec3 diffuseColor;
    float diffuseIntensity;
    //-------------------------- ( 16 bytes )
    
    float attConst;
    float attLin;
    float attQuad;
    //-------------------------- ( 16 bytes )
    
    //-------------------------- ( 4*16 = 64 bytes )
} pointlight;

layout(set = 0, binding = 2) uniform mvDirectionalLight
{

    vec3 viewLightDir;
    float  diffuseIntensity;
    //-------------------------- ( 16 bytes )
    
    vec3 diffuseColor;
    //-------------------------- ( 16 bytes )
    
    //-------------------------- ( 2*16 = 32 bytes )
} directionLight;

layout(set = 0, binding = 3) uniform sampler2D shadowMap;
layout(set = 0, binding = 4) uniform samplerCube shadowCubeMap;

layout(set = 1, binding = 0) uniform sampler2D colorSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;


layout(set = 1, binding = 3) uniform mvPhongMaterial
{
    vec3 materialColor;
    //-------------------------- ( 16 bytes )
    
    vec3 specularColor;
    float specularGloss;
    //-------------------------- ( 16 bytes )
   
    
    float normalMapWeight;
    bool useTextureMap;
    bool useNormalMap;
    bool useSpecularMap;
    //-------------------------- ( 16 bytes )
    
    
    bool useGlossAlpha;
    bool hasAlpha;
    bool doLighting;
    //-------------------------- ( 16 bytes )
    //-------------------------- ( 4 * 16 = 64 bytes )
};

float Attenuate(float attConst, float attLin, float attQuad, in float distFragToL)
{
    return 1.0 / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float textureProj(vec4 shadowCoord, vec2 off)
{

	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc, int range)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

vec3 calculateNormal()
{
    vec3 N = normalize(Normal);

    if(hasAlpha)
    {
        // flip normal when backface
        if (dot(ViewNormal, ViewPos) >= 0.0)
        {
            N = -N;
        }
    }    
    vec3 T = normalize(Tangent);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = transpose(mat3(T, B, N));
//    mat3 TBN = mat3(T, B, N);

    if(useNormalMap && scene.doNormal)
    {
        //vec3 tangentNormal = texture(normalSampler, UV).xyz;
        vec3 tangentNormal = texture(normalSampler, UV).xyz * 2.0 - 1.0;
        return normalize(TBN*tangentNormal);
    }

    return N;
}

void main() 
{

    if(!doLighting)
    {
        outColor = vec4(materialColor, 1.0);
        return;
    }

    vec4 materialColor = vec4(materialColor, 1.0);

    if(scene.doDiffuse)
    {
        if(useTextureMap) materialColor = texture(colorSampler, UV);
    }
    

    vec3 diffuse = { 0.0, 0.0, 0.0 };
    vec3 specularReflected = { 0.0, 0.0, 0.0 };
    vec3 specularReflectedColor = specularColor;

    if(hasAlpha)
    {
        if(materialColor.a < 0.1) discard;
    }

    vec3 N = calculateNormal();

    // specular parameter determination (mapped or uniform)
    float specularPowerLoaded = specularGloss;

    if(useSpecularMap)
    {
        if(scene.doSpecular)
        {
            const vec4 specularSample = texture(specularSampler, UV);
            specularReflectedColor = specularSample.rgb;
            if (useGlossAlpha)
            {
                specularPowerLoaded = pow(2.0, specularSample.a * 13.0);
            }
        }
    }

    //-----------------------------------------------------------------------------
    // point light
    //-----------------------------------------------------------------------------
    {
        // fragment to light vector data
        vec3 lightVec = pointlight.viewLightPos - WorldPos;
        float lightDistFromFrag = length(lightVec);
        vec3 lightDir = normalize(lightVec/lightDistFromFrag);
        float shadow = 1.0;

        if (scene.doOmniShadows)
        {
            float dist = length(lightVec);
            vec3 oshadowWorldPosMod = oshadowWorldPos.xyz;
            oshadowWorldPosMod.z = -oshadowWorldPos.z;
            float sampledDist = texture(shadowCubeMap, oshadowWorldPosMod).r;
            
	        // Check if fragment is in shadow
            shadow = (dist <= sampledDist - EPSILON) ? 1.0 : SHADOW_OPACITY;
        }
    
	    // attenuation
        const float att = Attenuate(pointlight.attConst, pointlight.attLin, pointlight.attQuad, lightDistFromFrag);
    
	    // diffuse
        diffuse += shadow * pointlight.diffuseColor * pointlight.diffuseIntensity * att * max(0.0, dot(lightDir, N));
    
        // specular
        const vec3 viewDir = normalize(scene.camPos - WorldPos);
        const vec3 r = reflect(-viewDir, N);

        float spec = pow(max(dot(viewDir, r), 0.0), specularPowerLoaded);
        //specularReflected += shadow * att * pointlight.diffuseColor * pointlight.diffuseIntensity * specularReflectedColor * spec;
    }

    //-----------------------------------------------------------------------------
    // directional light
    //-----------------------------------------------------------------------------
    {

        vec3 lightDir = -directionLight.viewLightDir;
        float shadow = 1.0;

        // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
        //if ((clamp(projectTexCoord.x, 0.0, 1.0) == projectTexCoord.x) && (clamp(projectTexCoord.y, 0.0, 1.0) == projectTexCoord.y) && scene.doDirectionalShadows)
        if (scene.doDirectionalShadows)
        {
            shadow = (scene.doPCF) ? filterPCF(dshadowWorldPos / dshadowWorldPos.w, scene.pcfRange) : textureProj(dshadowWorldPos / dshadowWorldPos.w, vec2(0.0));
        }

        // diffuse
        //diffuse += shadow * directionLight.diffuseColor * directionLight.diffuseIntensity * max(0.0, dot(normalize(lightDir), N));
    
        // specular
        
        // calculate reflected light vector
        const vec3 w = N * dot(normalize(lightDir), N);
        const vec3 r = normalize(w * 2.0 - normalize(lightDir));
        
        // vector from camera to fragment
        const vec3 viewCamToFrag = normalize(WorldPos - scene.camPos);
        
        //specularReflected += shadow * directionLight.diffuseColor * directionLight.diffuseIntensity * specularReflectedColor * 1.0 * pow(max(0.0, dot(-r, viewCamToFrag)), specularPowerLoaded);
        
    }

    outColor = clamp((vec4(diffuse, 1.0) + vec4(0.04, 0.04, 0.04, 1.0)) * materialColor + vec4(specularReflected, 1.0), 0.0, 1.0);   
}