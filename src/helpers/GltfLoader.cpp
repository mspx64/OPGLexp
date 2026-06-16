
#include "GltfLoader.h"
#include "Logger.h"

#include "Renderer/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <filesystem>
#include <iostream>

namespace loader::assimp {

bool ProcaessMaterials(const aiScene* scene, const std::string& dir) {

    for (int i = 0; i < scene->mNumMaterials; ++i) {

        auto* mat = scene->mMaterials[i];
        ASSERT(mat);
        auto it = lgt::g_MaterialBRDF.find(mat->GetName().C_Str());

        if (it != lgt::g_MaterialBRDF.end())
            continue;

        // TODO this is very poorly written code as i dont have much time
        //  refactor it
        aiTextureMapMode  mapMode[3];
        lgt::MaterialBRDF materialBrdf{};
        materialBrdf.textures.resize(1);
        materialBrdf.textures[0].type      = lgt::TextureType::BASE_COLOR;
        materialBrdf.textures[0].textureId = LoadTexture(mat, scene, mapMode, aiTextureType_BASE_COLOR, dir);
        if (materialBrdf.textures[0].textureId != std::string("INVALID_TEXTURE")) {
            materialBrdf.textures[0].textureHandle         = lgt::GetTexture(materialBrdf.textures[0].textureId).handle;
            materialBrdf.textures[0].samplerState.wrapMode = MapAssimpWrapMode(mapMode[0]);

            // defaults
            materialBrdf.textures[0].samplerState.minFilter = GL_LINEAR_MIPMAP_LINEAR;
            materialBrdf.textures[0].samplerState.magFilter = GL_LINEAR;

            materialBrdf.textures[0].bindlessHandle = lgt::GetBindlessTextureSamplerHandle(materialBrdf.textures[0].textureHandle,
                                                                                           materialBrdf.textures[0].samplerState);

            // add it to the global cpu side gpu material buffer
            materialBrdf.gpuIndex = lgt::g_MaterialGPU.size();
            lgt::g_MaterialGPU.push_back(std::move(materialBrdf.ToMaterialGPU()));

            // add it to the globle cpu Material storage
            lgt::g_MaterialBRDF[mat->GetName().C_Str()] = std::move(materialBrdf);
        }
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

    if (mat->GetTexture(type, 0, &textureName, nullptr, nullptr, nullptr, nullptr, mapMode) != AI_SUCCESS) {
        CORE_ERROR("Material {} has no texture of type {}", mat->GetName().C_Str(), TextureTypeToString(type));
        return "INVALID_TEXTURE";
    }

    texturePath = std::filesystem::path(dir) / std::filesystem::path(textureName.C_Str()).filename();
    auto it     = lgt::g_Textures.find(texturePath.string());
    if (it != lgt::g_Textures.end())
        return texturePath.string();

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
        data = stbi_load(texturePath.string().c_str(), &width, &height, &channels, 4);
    }

    if (data) {
        lgt::TextureDesc desc{};
        desc.data     = data;
        desc.width    = width;
        desc.height   = height;
        desc.channels = 4;

        lgt::Texture new_texture              = lgt::CreateTexture(desc);
        lgt::g_Textures[texturePath.string()] = std::move(new_texture);

        stbi_image_free(data);
        CORE_INFO("Loaded Texture {}", texturePath.string());
        return texturePath.string();
    }

    CORE_ERROR("stb_image : failed to laod the texture -> {}", texturePath.string());
    return std::string("INVALID_TEXTURE");
}

} // namespace loader::assimp
