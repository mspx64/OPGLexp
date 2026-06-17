#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include "Texture.h"

namespace lgt {

struct alignas(16) MaterialGPU {
    glm::vec4 baseColor;
    glm::vec4 emmisiveColor;

    float roughness;
    float metallic;
    float emmisiveStrength;

    GLuint64 normalMap;
    GLuint64 diffuseMap;
    GLuint64 emmisiveMap;
};

struct MaterialTextureAccess {
    TextureType  type;
    std::string  textureId;
    GLuint       textureHandle;
    GLuint64     bindlessHandle;
    SamplerState samplerState;
};

struct MaterialBRDF {
    uint32_t  gpuIndex = 0;
    glm::vec4 baseColor;
    glm::vec4 emmisiveColor;
    float     roughness;
    float     metallic;
    float     emmisiveStrength;

    std::vector<MaterialTextureAccess> textures;

    MaterialGPU ToMaterialGPU() {
        MaterialGPU result{};

        result.baseColor     = baseColor;
        result.emmisiveColor = emmisiveColor;

        result.roughness        = roughness;
        result.metallic         = metallic;
        result.emmisiveStrength = emmisiveStrength;

        for (auto& texAccess : textures) {
            switch (texAccess.type) {
            case TextureType::BASE_COLOR:
                result.diffuseMap = texAccess.bindlessHandle;
                break;
            case TextureType::NORMAL:
                result.normalMap = texAccess.bindlessHandle;
                break;
            case TextureType::EMISSIVE:
                result.emmisiveMap = texAccess.bindlessHandle;
                break;
            default:
                break;
            }
        }
        return result;
    }
};

extern GLuint                                         g_MaterialSSBO;
extern std::vector<MaterialGPU>                       g_MaterialGPU;
extern std ::unordered_map<std::string, MaterialBRDF> g_MaterialBRDF;

void UpateMaterial(uint32_t index, MaterialGPU data);

} // namespace lgt