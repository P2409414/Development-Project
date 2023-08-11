//#version 410 core

//out vec4 FragColor;

//uniform sampler2D texture_diffuse1;

//in vec3 vNorm;
//in vec2 vUV; 

//void main()
//{    
//	FragColor = texture(texture_diffuse1, vUV);
	//FragColor = vec4(0.5,0.5,0.5,1.0f) ;
	
//}



#version 410 core

vec3 getDirectionalLight(vec3 norm, vec3 viewDir);
vec3 getPointLight(vec3 norm, vec3 viewDir);
vec3 getSpotLight(vec3 norm, vec3 viewDir);

//layout (location = 0) out vec4 FragColor;
//layout (location = 1) out vec4 brightColour;
out vec4 FragColor;

uniform sampler2D texture_diffuse1;

in vec3 posWS;
in vec3 vNorm;
in vec2 vUV; 

#define numPointLights 2

struct pointLight{
    vec3 position;
    vec3 colour;
    float Kc;
    float Kl;
    float Ke;
};
struct spotLight{
    vec3 position;
    vec3 direction;
    vec3 colour;
    float Kc;
    float Kl;
    float Ke;

    float innerRad;
    float outerRad;
};

uniform float bloomBrightness;
uniform vec3 lightCol;
uniform vec3 lightDir;
uniform vec3 objectCol;
uniform vec3 viewPos;
uniform pointLight pLight[numPointLights];
uniform spotLight sLight;
//uniform sampler2D normalMap;
//uniform sampler2D diffuse;
//uniform sampler2D specular;

float ambientFactor = 1.0f;
float shine = 0.2f;
float specularStrength = 0.2f;

void main()
{
    vec3 norm = vec3(0.0);
    norm = vec3(texture(texture_diffuse1, vUV).xyz);
    norm = norm*2.0 - 1.0;
    norm = normalize(norm);
    vec3 viewDir = normalize(viewPos - posWS);
    vec3 result = getDirectionalLight(norm, viewDir);
    result = result + getPointLight(norm, viewDir);
    result = result + getSpotLight(norm, viewDir);
    

    FragColor = vec4(result, 1.0);
}

vec3 getDirectionalLight(vec3 norm, vec3 viewDir){
    vec3 diffMapColour = vec3(texture(texture_diffuse1, vUV).xyz);
    vec3 specMapColour = vec3(texture(texture_diffuse1, vUV).x);
    vec3 ambientColour = lightCol*diffMapColour*ambientFactor;
    float diffuseFactor = dot(norm, -lightDir);
    diffuseFactor = max(diffuseFactor,0.0);
    vec3 diffuseColour = lightCol*diffMapColour*diffuseFactor;
    vec3 halfwayDir = -lightDir + viewDir;
    float specularFactor = dot(halfwayDir, norm);
    specularFactor = max(specularFactor,0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColour = lightCol * specMapColour * specularFactor;
    vec3 result = ambientColour + diffuseColour + specularColour;

    return result;
}

vec3 getPointLight(vec3 norm, vec3 viewDir){
    vec3 diffMapColour = vec3(texture(texture_diffuse1, vUV).xyz);
    vec3 specMapColour = vec3(texture(texture_diffuse1, vUV).x);
    vec3 result = vec3(0.0);
    for (int i = 0; i < numPointLights; i++){
    float dist = length(pLight[i].position - posWS);
    float attn = 1.0/(pLight[i].Kc + (pLight[i].Kl*dist) + (pLight[i].Ke*(dist*dist)));
    vec3 pLightDir = normalize(pLight[i].position - posWS);

    vec3 ambientColour = pLight[i].colour*diffMapColour*ambientFactor;
    ambientColour = ambientColour * attn;

    float diffuseFactor = dot(norm, pLightDir);
    diffuseFactor = max(diffuseFactor,0.0);
    vec3 diffuseColour = pLight[i].colour*diffMapColour*diffuseFactor;
    diffuseColour = diffuseColour * attn;

    vec3 halfwayDir = -pLightDir + viewDir;
    float specularFactor = dot(halfwayDir, norm);
    specularFactor = max(specularFactor,0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColour = pLight[i].colour * specMapColour * specularFactor;
    specularColour = specularColour * attn;
    vec3 pointLightResult = ambientColour + diffuseColour + specularColour;
    result = result + pointLightResult;
    };

    return result;
}

vec3 getSpotLight(vec3 norm, vec3 viewDir){
    vec3 diffMapColour = vec3(texture(texture_diffuse1, vUV).xyz);
    vec3 specMapColour = vec3(texture(texture_diffuse1, vUV).x);
    vec3 result = vec3(0.0);
    float dist = length(sLight.position - posWS);
    float attn = 1.0/(sLight.Kc + (sLight.Kl*dist) + (sLight.Ke*(dist*dist)));
    vec3 sLightDir = normalize(sLight.position - posWS);

    float diffuseFactor = dot(norm, sLightDir);
    diffuseFactor = max(diffuseFactor,0.0);
    vec3 diffuseColour = sLight.colour*diffMapColour*diffuseFactor;
    diffuseColour = diffuseColour * attn;

    vec3 halfwayDir = -sLightDir + viewDir;
    float specularFactor = dot(norm, halfwayDir);
    specularFactor = max(specularFactor,0.0);
    specularFactor = pow(specularFactor, shine);
    vec3 specularColour = sLight.colour * specMapColour * specularFactor;
    specularColour = specularColour * attn;
    

    float theta = dot(-sLightDir, normalize(sLight.direction));
    float illum = (theta - sLight.outerRad) / (sLight.innerRad - sLight.outerRad);
    illum = clamp(illum, 0.0, 1.0);
    diffuseColour = diffuseColour * illum;
    specularColour = specularColour * illum;
    result = diffuseColour + specularColour;

    return result;
}