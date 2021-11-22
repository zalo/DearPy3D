#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inViewPos;
layout(location = 1) in vec3 inViewNormal;
layout(location = 2) in vec3 inWorldNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 indshadowWorldPos;
layout(location = 5) out vec4 inoshadowWorldPos;
layout(location = 6) in mat3 inTangentBasis;


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
        if(useTextureMap) materialColor = texture(colorSampler, inTexCoord);
    }
    

    vec3 diffuse = { 0.0, 0.0, 0.0 };
    vec3 specularReflected = { 0.0, 0.0, 0.0 };
    vec3 specularReflectedColor = specularColor;
    vec3 viewNormal = inViewNormal;

    if(hasAlpha)
    {
        if(materialColor.a < 0.1) discard;

        // flip normal when backface
        if (dot(viewNormal, inViewPos) >= 0.0)
        {
            viewNormal = -inViewNormal;
        }
    }

    // normalize the mesh normal
    viewNormal = normalize(viewNormal);

    if(useNormalMap)
    {
        if(scene.doNormal)
        {
            const vec4 normalSample = texture(normalSampler, inTexCoord);
            const vec4 tanNormal = normalSample * 2.0 - 1.0;
            const vec3 mappedNormal = normalize(inTangentBasis * tanNormal.rgb);
            viewNormal = mix(viewNormal, mappedNormal, normalMapWeight);
        }
    }

    // specular parameter determination (mapped or uniform)
    float specularPowerLoaded = specularGloss;

    if(useSpecularMap)
    {
        if(scene.doSpecular)
        {
            const vec4 specularSample = texture(specularSampler, inTexCoord);
            specularReflectedColor = specularSample.rgb;
            //specularReflectedColor = vec3(specularSample.x, specularSample.y, specularSample.z);
        
            if (useGlossAlpha)
            {
                specularPowerLoaded = pow(2.0, specularSample.a * 13.0f);
            }
        }
    }

    //-----------------------------------------------------------------------------
    // point light
    //-----------------------------------------------------------------------------
    {
        // fragment to light vector data
        vec3 lightVec = pointlight.viewLightPos - inViewPos;
        float lightDistFromFrag = length(lightVec);
        vec3 lightDirVec = lightVec / lightDistFromFrag;
    
	    // attenuation
        const float att = Attenuate(pointlight.attConst, pointlight.attLin, pointlight.attQuad, lightDistFromFrag);
    
	    // diffuse
        diffuse += pointlight.diffuseColor * pointlight.diffuseIntensity * att * max(0.0, dot(lightDirVec, viewNormal));
    
        // specular
        
        // calculate reflected light vector
        const vec3 w = viewNormal * dot(lightVec, viewNormal);
        const vec3 r = normalize(w * 2.0 - lightVec);
        
        // vector from camera to fragment
        const vec3 viewCamToFrag = normalize(inViewPos);
        
        specularReflected += att * pointlight.diffuseColor * pointlight.diffuseIntensity * specularReflectedColor * 1.0 * pow(max(0.0, dot(-r, viewCamToFrag)), specularPowerLoaded);
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
            shadow = (scene.doPCF) ? filterPCF(indshadowWorldPos / indshadowWorldPos.w, scene.pcfRange) : textureProj(indshadowWorldPos / indshadowWorldPos.w, vec2(0.0));
        }

        // diffuse
        diffuse += shadow * directionLight.diffuseColor * directionLight.diffuseIntensity * max(0.0, dot(normalize(lightDir), viewNormal));
    
        // specular
        
        // calculate reflected light vector
        const vec3 w = viewNormal * dot(normalize(lightDir), viewNormal);
        const vec3 r = normalize(w * 2.0 - normalize(lightDir));
        
        // vector from camera to fragment
        const vec3 viewCamToFrag = normalize(inViewPos);
        
        specularReflected += shadow * directionLight.diffuseColor * directionLight.diffuseIntensity * specularReflectedColor * 1.0 * pow(max(0.0, dot(-r, viewCamToFrag)), specularPowerLoaded);
        
    }

    outColor = clamp((vec4(diffuse, 1.0) + vec4(0.04, 0.04, 0.04, 1.0)) * materialColor + vec4(specularReflected, 1.0), 0.0, 1.0);   
}