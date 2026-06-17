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
    vec4 baseColor;
    vec4 emmisiveColor;

    float roughness;
    float metallic;
    float emmisiveStrength;

    sampler2D  normalMap;
    sampler2D  diffuseMap;
    sampler2D  emmisiveMap;
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


float GGX( float NdotH , float roughness){
    float roughness2 = roughness * roughness ;   
    float denom = ((NdotH * NdotH * (roughness2 - 1.0)) + 1.0);
    return roughness2/ (3.14159265 * denom * denom);
}

float Fschlick(float F0, float HdotV) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

float DisneyDiffuse(float NdotL , float LdotV ,float LdotH, float roughness ){
float F90 = 0.5 + 2 * (roughness * LdotH * LdotH);
float F0 = 1.0f;
float diffuse = mix(F0 , F90 ,NdotL) * mix(F0 , F90 ,LdotV);
return  diffuse/3.14159265;
}

float SmithGGX(float NdotV, float NdotL, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float GGXL = NdotV * sqrt(a2 + NdotL * (NdotL - a2 * NdotL));
    float GGXV = NdotL * sqrt(a2 + NdotV * (NdotV - a2 * NdotV));

    return 0.5 / (GGXV + GGXL);
}

void main(){

    float roughness = clamp(materials[u_MaterialIndex].roughness, 0.05, 1.0);
    float metallic  = materials[u_MaterialIndex].metallic;

    vec4 albedo   = texture(materials[u_MaterialIndex].diffuseMap, Textcoord) * materials[u_MaterialIndex].baseColor;
    vec4 emmisive = texture(materials[u_MaterialIndex].emmisiveMap, Textcoord) * materials[u_MaterialIndex].emmisiveColor;

    vec3 normal = texture(materials[u_MaterialIndex].normalMap, Textcoord).rgb;

    vec3 N = normalize(normal * 2.0 - 1.0);
    vec3 L = normalize(TangentLightDir);
    vec3 V = normalize(TangentViewDir);
    vec3 H = normalize(L + V);

    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float LdotV = clamp(dot(L, V), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);

    float D = GGX(NdotH, roughness);
    float G = SmithGGX(NdotV, NdotL, roughness);

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic); 
    vec3 F  = vec3(Fschlick(F0.r, LdotH)); 

    vec3 specularTerm = D * G * F; 
    float diff = DisneyDiffuse(NdotL, NdotV, LdotH, roughness);
    vec3 diffuseTerm = albedo.rgb * diff;

    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 finalColor = (kd * diffuseTerm + specularTerm) * lightColor * NdotL;

    finalColor += emmisive.rgb * materials[u_MaterialIndex].emmisiveStrength;
    out_color = vec4(finalColor, albedo.a);
}
