
#include "GltfLoader.h"
#include "Logger.h"

#include "Renderer/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <filesystem>
#include <iostream>

namespace loader::assimp {

struct TextureMapping {
    aiTextureType    assimpType;
    lgt::TextureType engineType;
};

const std::vector<TextureMapping> g_PBRMappings = {{aiTextureType_BASE_COLOR, lgt::TextureType::BASE_COLOR},
                                                   {aiTextureType_NORMALS, lgt::TextureType::NORMAL},
                                                   {aiTextureType_METALNESS, lgt::TextureType::METALNESS},
                                                   {aiTextureType_DIFFUSE_ROUGHNESS, lgt::TextureType::ROUGHNESS},
                                                   {aiTextureType_AMBIENT_OCCLUSION, lgt::TextureType::AMBIENT_OCCLUSION},
                                                   {aiTextureType_NORMAL_CAMERA, lgt::TextureType::NORMAL},
                                                   {aiTextureType_EMISSIVE, lgt::TextureType::EMISSIVE}};

bool ProcaessMaterials(const aiScene* scene, const std::string& dir) {
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        auto* mat = scene->mMaterials[i];
        ASSERT(mat);

        auto it = lgt::g_MaterialBRDF.find(mat->GetName().C_Str());
        if (it != lgt::g_MaterialBRDF.end())
            continue;

        lgt::MaterialBRDF materialBrdf{};

        aiColor4D color;
        float     value;

        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            materialBrdf.baseColor = glm::vec4(color.r, color.g, color.b, color.a);
        } else {
            materialBrdf.baseColor = glm::vec4(1.0f);
        }

        if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
            materialBrdf.emmisiveColor = glm::vec4(color.r, color.g, color.b, color.a);
        } else {
            materialBrdf.emmisiveColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }

        if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS) {
            materialBrdf.roughness = value;
        } else {
            materialBrdf.roughness = 0.5f;
        }

        if (mat->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS) {
            materialBrdf.metallic = value;
        } else {
            materialBrdf.metallic = 0.0f;
        }

        if (mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, value) == AI_SUCCESS) {
            materialBrdf.emmisiveStrength = value;
        } else {
            materialBrdf.emmisiveStrength = 0.0f;
        }

        for (const auto& mapping : g_PBRMappings) {
            if (mat->GetTextureCount(mapping.assimpType) > 0) {
                aiTextureMapMode mapModes[3] = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};
                std::string      textureId   = LoadTexture(mat, scene, mapModes, mapping.assimpType, dir);

                if (textureId != "INVALID_TEXTURE") {
                    lgt::MaterialTextureAccess texInfo{};
                    texInfo.type          = mapping.engineType;
                    texInfo.textureId     = textureId;
                    texInfo.textureHandle = lgt::GetTexture(textureId).handle;

                    texInfo.samplerState.wrapModeS = MapAssimpWrapMode(mapModes[0]);
                    texInfo.samplerState.wrapModeT = MapAssimpWrapMode(mapModes[1]);
                    texInfo.samplerState.minFilter = GL_LINEAR_MIPMAP_LINEAR;
                    texInfo.samplerState.magFilter = GL_LINEAR;

                    texInfo.bindlessHandle = lgt::GetBindlessTextureSamplerHandle(texInfo.textureHandle, texInfo.samplerState);
                    materialBrdf.textures.push_back(std::move(texInfo));
                }
            }
        }

        materialBrdf.gpuIndex = lgt::g_MaterialGPU.size();
        lgt::g_MaterialGPU.push_back(std::move(materialBrdf.ToMaterialGPU()));
        lgt::g_MaterialBRDF[mat->GetName().C_Str()] = std::move(materialBrdf);
    }

    return scene->HasMaterials();
}
static const char* TextureTypeToString(aiTextureType type) {
    switch (type) {
    case aiTextureType_NONE:
        return "NONE";
    case aiTextureType_DIFFUSE:
        return "DIFFUSE";
    case aiTextureType_SPECULAR:
        return "SPECULAR";
    case aiTextureType_AMBIENT:
        return "AMBIENT";
    case aiTextureType_EMISSIVE:
        return "EMISSIVE";
    case aiTextureType_HEIGHT:
        return "HEIGHT";
    case aiTextureType_NORMALS:
        return "NORMALS";
    case aiTextureType_SHININESS:
        return "SHININESS";
    case aiTextureType_OPACITY:
        return "OPACITY";
    case aiTextureType_DISPLACEMENT:
        return "DISPLACEMENT";
    case aiTextureType_LIGHTMAP:
        return "LIGHTMAP";
    case aiTextureType_REFLECTION:
        return "REFLECTION";
    case aiTextureType_BASE_COLOR:
        return "BASE_COLOR";
    case aiTextureType_NORMAL_CAMERA:
        return "NORMAL_CAMERA";
    case aiTextureType_EMISSION_COLOR:
        return "EMISSION_COLOR";
    case aiTextureType_METALNESS:
        return "METALNESS";
    case aiTextureType_DIFFUSE_ROUGHNESS:
        return "DIFFUSE_ROUGHNESS";
    case aiTextureType_AMBIENT_OCCLUSION:
        return "AMBIENT_OCCLUSION";
    case aiTextureType_SHEEN:
        return "SHEEN";
    case aiTextureType_CLEARCOAT:
        return "CLEARCOAT";
    case aiTextureType_TRANSMISSION:
        return "TRANSMISSION";
    case aiTextureType_UNKNOWN:
        return "UNKNOWN";

    default:
        return "INVALID";
    }
}

std::string
LoadTexture(aiMaterial* mat, const aiScene* scene, aiTextureMapMode* mapMode, aiTextureType type, const std::string& dir) {

    aiString              textureName;
    void*                 data  = nullptr;
    int                   width = 0, height = 0, channels;
    std::filesystem::path texturePath;
    std::string           textureId;

    if (mat->GetTexture(type, 0, &textureName, nullptr, nullptr, nullptr, nullptr, mapMode) != AI_SUCCESS) {
        CORE_ERROR("Material {} has no texture of type {}", mat->GetName().C_Str(), TextureTypeToString(type));
        return "INVALID_TEXTURE";
    }

    std::filesystem::path rawPath(textureName.C_Str());

    if (rawPath.is_absolute()) {
        texturePath = rawPath;
    } else {
        texturePath = std::filesystem::path(dir) / rawPath;
    }
    textureId = texturePath.lexically_normal().string();

    auto it = lgt::g_Textures.find(textureId);
    if (it != lgt::g_Textures.end())
        return textureId;

    const aiTexture* embeddedTex = scene->GetEmbeddedTexture(textureName.C_Str());

    if (embeddedTex) {
        if (embeddedTex->mHeight == 0) { // Compressed format (png/jpg)
            data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(embeddedTex->pcData), embeddedTex->mWidth, &width, &height, &channels, 4);
        } else {
            data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(embeddedTex->pcData),
                                         embeddedTex->mWidth * embeddedTex->mHeight,
                                         &width,
                                         &height,
                                         &channels,
                                         4);
        }
    } else {
        data = stbi_load(textureId.c_str(), &width, &height, &channels, 4);
    }

    if (data) {
        lgt::TextureDesc desc{};
        desc.data         = data;
        desc.width        = width;
        desc.height       = height;
        desc.generateMips = true;
        desc.channels     = 4;

        lgt::Texture new_texture   = lgt::CreateTexture(desc);
        lgt::g_Textures[textureId] = std::move(new_texture);

        stbi_image_free(data);
        CORE_INFO("Loaded Texture {}", textureId);
        return textureId;
    }

    CORE_ERROR("stb_image : failed to laod the texture -> {}", textureId);
    return "INVALID_TEXTURE";
}

} // namespace loader::assimp
