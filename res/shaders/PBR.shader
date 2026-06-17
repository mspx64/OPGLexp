#shader Vertex
#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textcoord;
layout(location = 3) in vec4 tangent;

layout(location = 0) out vec3 TangentLightDir;
layout(location = 1) out vec3 TangentViewDir;
layout(location = 2) out vec2 Textcoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPos; 

vec3 lightPos = vec3(1.0f, 1.0f, 1.0f);

void main (){
    vec3 T = normalize(mat3(u_Model) * tangent.xyz);
    vec3 N = normalize(mat3(u_Model) * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * tangent.w;

    mat3 worldToTangent = transpose(mat3(T, B, N));

    vec3 fragpos = vec3(u_Model * vec4(pos, 1.0));

    vec3 worldLightDir = normalize(lightPos - fragpos);
    vec3 worldViewDir  = normalize(u_CameraPos - fragpos);

    TangentLightDir = worldToTangent * worldLightDir;
    TangentViewDir  = worldToTangent * worldViewDir;
    Textcoord = textcoord;

    gl_Position = u_Projection * u_View * vec4(fragpos, 1.0);
}

#shader Fragment
#version 460 core
#extension GL_ARB_bindless_texture : require

vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

struct Material {
    vec4 specularColor;
    vec4 diffuseColor;
    vec4 emmisiveCOlor; // Fixed typo if matching C++
    vec4 baseColor;
    float roughness;
    float metalic;
    sampler2D normalMap;
    sampler2D diffuseMap;
    sampler2D metallicRoughnessMap;
};

layout(location = 0) in vec3 TangentLightDir;
layout(location = 1) in vec3 TangentViewDir;
layout(location = 2) in vec2 Textcoord;

layout(location = 0) out vec4 out_color;

layout(std430 , binding = 0) buffer Materials{
    Material materials[];
};

uniform int u_MaterialIndex;
uniform int u_DebugMode;


float GGX(float roughness, float cosNH){
    float roughness2 = roughness * roughness ;   
    float denom = ((cosNH * cosNH * (roughness2 - 1.0)) + 1.0);
    float result = roughness2/ (3.14159265 * denom * denom);
    return result;
}

float Fschlick(float F0 , float F90 , float cosTheta){
    return mix(F0 , F90 , pow(1.0f - cosTheta ,5));
}

float DisneyDiffuse(float roughness , vec3 L , vec3 V , vec3 N , vec3 H){
//TODO    
return 0.0f ;
}

 float GeometrySmith_HeightCorrelated(float NdotV, float NdotL, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);

    return 0.5 / (GGXV + GGXL);
}

void main(){
    vec4 basetexture = texture(materials[u_MaterialIndex].diffuseMap, Textcoord);
    
    vec3 normaltexture = texture(materials[u_MaterialIndex].normalMap, Textcoord).rgb;

    vec3 N = normalize(normaltexture * 2.0 - 1.0);
    vec3 L = normalize(TangentLightDir);
    vec3 V = normalize(TangentViewDir);
    vec3 H = normalize(L + V);

    /*NdotL  , NdotH , NdotV*/
    float NdotL = dot(N, L);
    float NdotH = dot(N, H);
    float NdotV = dot(N,V);

    // float specular =  GGX(materials[u_MaterialIndex].roughness, cosNH);
    // out_color = vec4((basetexture.rgb * cosNL) + (vec3(specular) * 0.1), basetexture.a);
     out_color = vec4( vec3(GeometrySmith_HeightCorrelated(NdotV , NdotL ,materials[u_MaterialIndex].roughness ) ), 1.0f );
}
