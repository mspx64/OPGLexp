#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include "Texture.h"

namespace lgt {

struct alignas(16) MaterialGPU {

    glm::vec4 specularColor;
    glm::vec4 diffuseColor;
    glm::vec4 emmisiveCOlor;
    glm::vec4 baseColor;

    float tansperancyFactor;
    float alphaTest;

    GLuint64 normalMap;
    GLuint64 baseColorMap;
    GLuint64 metallicRoughnessMap;
};

struct MaterialTextureAccess {
    TextureType  type;
    std::string  textureId;
    GLuint       textureHandle;
    GLuint64     bindlessHandle;
    SamplerState samplerState;
};

struct MaterialBRDF {
    uint32_t gpuIndex = 0;
    /*TODO add the remaining PBR parrameters*/
    std::vector<MaterialTextureAccess> textures;

    MaterialGPU ToMaterialGPU() {
        MaterialGPU result{};
        for (auto& texAccess : textures) {
            switch (texAccess.type) {
            case TextureType::BASE_COLOR:
                result.baseColorMap = texAccess.bindlessHandle;
                break;

            case TextureType::NORMAL:
                result.normalMap = texAccess.bindlessHandle;
                break;
            // TODO add the remaining textures
            default:
                break;
            }
        }
        return result;
    }
};

extern std::vector<MaterialGPU>                       g_MaterialGPU;
extern std ::unordered_map<std::string, MaterialBRDF> g_MaterialBRDF;

void UpateMaterial(uint32_t index, MaterialGPU data);

} // namespace lgt