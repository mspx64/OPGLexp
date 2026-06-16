#pragma once
#include "Renderer/Texture.h"

#include <assimp/scene.h>

namespace loader::assimp {

// Map Assimp wrap modes to OpenGL
inline GLenum MapAssimpWrapMode(aiTextureMapMode mode) {
    switch (mode) {
    case aiTextureMapMode_Wrap:
        return GL_REPEAT;

    case aiTextureMapMode_Clamp:
        return GL_CLAMP_TO_EDGE;

    case aiTextureMapMode_Mirror:
        return GL_MIRRORED_REPEAT;

    default:
        return GL_REPEAT;
    }
}

// TODO
//  here we are assuming that  , engine will only load  one gltf scene at a time
//  if not the material indicies in the gloabal material buffer get scarambled
//  it a simple fix but i want to foucse on to get the pbr working
bool ProcaessMaterials(const aiScene* scene, const std::string& dir);
std::string
LoadTexture(aiMaterial* mat, const aiScene* scene, aiTextureMapMode* mapMode, aiTextureType type, const std::string& dir);
} // namespace loader::assimp