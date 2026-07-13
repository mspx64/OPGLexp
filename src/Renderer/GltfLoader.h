#pragma once
#include "Renderer/Texture.h"

#include <assimp/scene.h>

namespace loader::assimp {



// TODO
//  here we are assuming that  , engine will only load  one gltf scene at a time
//  if not , the material indicies in the gloabal material buffer will get scarambled
//  it's a simple fix but i want to focuse on to get the pbr

std::vector<uint32_t> ProcaessMaterials(const aiScene* scene, const std::string& dir);
std::string
LoadTexture(aiMaterial* mat, const aiScene* scene, aiTextureMapMode* mapMode, aiTextureType type, const std::string& dir);
} // namespace loader::assimp
