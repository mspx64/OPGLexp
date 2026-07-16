#shader Vertex
#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textcoord;
layout(location = 3) in vec4 tangent;

layout(location = 0) out vec3 WorldPos;
layout(location = 1) out vec2 Textcoord;
layout(location = 2) out mat3 TBN;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    vec3 T = normalize(mat3(u_Model) * tangent.xyz);
    vec3 N = normalize(mat3(u_Model) * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * tangent.w;

    TBN = mat3(T, B, N);

    WorldPos = vec3(u_Model * vec4(pos, 1.0));
    Textcoord = textcoord;

    gl_Position = u_Projection * u_View * vec4(WorldPos, 1.0);
}

#shader Fragment
#version 460 core
#extension GL_ARB_bindless_texture : require

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

struct PointLight {
    vec4 position; // w is radius
    vec4 color;    // w is intensity
    float radius;
    float padding[3];
};

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec2 Textcoord;
layout(location = 2) in mat3 TBN;

layout(location = 0) out vec4 out_color;

layout(std430, binding = 0) buffer Materials {
    Material materials[];
};

layout(std430, binding = 1) readonly buffer LightBuffer {
    PointLight lights[];
};

layout(std430, binding = 2) readonly buffer VisibleLightIndicesBuffer {
    int visibleLightIndices[];
};

uniform int u_MaterialIndex;
uniform int u_DebugMode;
uniform vec3 u_CameraPos;
uniform int u_ScreenWidth;

float GGX(float NdotH, float roughness) {
    float roughness2 = roughness * roughness;
    float denom = ((NdotH * NdotH * (roughness2 - 1.0)) + 1.0);
    return roughness2 / (3.14159265 * denom * denom);
}

float Fschlick(float F0, float HdotV) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

vec3 FschlickVec3(vec3 F0, float HdotV) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

float DisneyDiffuse(float NdotL, float LdotV, float LdotH, float roughness) {
    float F90 = 0.5 + 2.0 * (roughness * LdotH * LdotH);
    float F0 = 1.0;
    float diffuse = mix(F0, F90, NdotL) * mix(F0, F90, LdotV);
    return diffuse / 3.14159265;
}

float SmithGGX(float NdotV, float NdotL, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float GGXL = NdotV * sqrt(a2 + NdotL * (NdotL - a2 * NdotL));
    float GGXV = NdotL * sqrt(a2 + NdotV * (NdotV - a2 * NdotV));

    if (GGXV + GGXL == 0.0) return 0.0;
    return 0.5 / (GGXV + GGXL);
}

void main() {
    float roughness = clamp(materials[u_MaterialIndex].roughness, 0.05, 1.0);
    float metallic  = materials[u_MaterialIndex].metallic;

    vec4 albedo   = texture(materials[u_MaterialIndex].diffuseMap, Textcoord) * materials[u_MaterialIndex].baseColor;
    vec4 emmisive = texture(materials[u_MaterialIndex].emmisiveMap, Textcoord) * materials[u_MaterialIndex].emmisiveColor;

    vec3 normalMap = texture(materials[u_MaterialIndex].normalMap, Textcoord).rgb;
    vec3 N = normalize(TBN * (normalMap * 2.0 - 1.0));
    vec3 V = normalize(u_CameraPos - WorldPos);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic); 

    // Tile look up for lights
    ivec2 tileID = ivec2(gl_FragCoord.xy) / 16;
    int numTilesX = (u_ScreenWidth + 15) / 16;
    uint tileIndex = tileID.y * numTilesX + tileID.x;
    uint offset = tileIndex * 257;
    int lightCount = visibleLightIndices[offset];

    vec3 finalColor = vec3(0.0);
    
    // Fallback directional light or ambient if no lights
    vec3 ambient = albedo.rgb * 0.03;
    finalColor += ambient;

    for (int i = 0; i < lightCount; i++) {
        int lightIdx = visibleLightIndices[offset + 1 + i];
        PointLight light = lights[lightIdx];

        vec3 L = light.position.xyz - WorldPos;
        float dist = length(L);
        L = normalize(L);
        vec3 H = normalize(L + V);

        float attenuation = clamp(1.0 - (dist * dist) / (light.radius * light.radius), 0.0, 1.0);
        attenuation *= attenuation;
        
        vec3 radiance = light.color.rgb * light.color.a * attenuation;

        float NdotL = clamp(dot(N, L), 0.0, 1.0);
        float NdotH = clamp(dot(N, H), 0.0, 1.0);
        float LdotV = clamp(dot(L, V), 0.0, 1.0);
        float LdotH = clamp(dot(L, H), 0.0, 1.0);

        float D = GGX(NdotH, roughness);
        float G = SmithGGX(NdotV, NdotL, roughness);
        vec3 F  = FschlickVec3(F0, LdotH); 

        vec3 specularTerm = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001); 
        float diff = DisneyDiffuse(NdotL, NdotV, LdotH, roughness);
        vec3 diffuseTerm = albedo.rgb * diff;

        vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);
        finalColor += (kd * diffuseTerm + specularTerm) * radiance * NdotL;
    }

    finalColor += emmisive.rgb * materials[u_MaterialIndex].emmisiveStrength;

    // Linear to Gamma
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    switch(u_DebugMode){
        case 0: out_color = vec4(albedo.rgb, 1.0); break;
        case 1: out_color = vec4(normalMap, 1.0); break;
        case 2: out_color = vec4(emmisive.rgb, 1.0); break;
        case 3: out_color = vec4(N * 0.5 + 0.5, 1.0); break;
        case 10: out_color = vec4(finalColor, albedo.a); break;
        default: out_color = vec4(finalColor, albedo.a); break;
    }
}
