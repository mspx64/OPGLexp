#include "Scene.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

#include "helpers/GltfLoader.h"
#include "helpers/Logger.h"

namespace lgt {

// Helper to convert Assimp's 4x4 matrix format directly to GLM row/column
// layouts
inline glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& from) {
    glm::mat4 to;
    // Assimp is row-major, GLM is column-major
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

void Scene::Clear() {
    m_RootNodes.clear();
}

void Scene::Update() {
    for (auto root : m_RootNodes) {
        root->UpdateTransformCascades();
    }
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

void Scene::processMaterials(const aiScene* scene, const std::string& dir) {

    m_materialBuffer.resize(scene->mNumMaterials);

    for (unsigned int i = 0; i < 1; i++) {

        aiMaterial* mat = scene->mMaterials[i];

        for (int type = 0; type <= aiTextureType_UNKNOWN; type++) {
            aiTextureType texType = (aiTextureType)type;

            unsigned int count = mat->GetTextureCount(texType);

            if (count == 0)
                continue;

            aiString path;

            mat->GetTexture(texType, 0, &path);

            CORE_INFO("Type :{} -> path : {}", std::string(TextureTypeToString(texType)), path.C_Str());
        }

        MaterialGPU& gpuMat = m_materialBuffer[i];

        aiColor4D baseColor;
        if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &baseColor)) {
            gpuMat.baseColor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
        }

        gpuMat.baseColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

        auto textureid = loader::assimp::LoadTexture(mat, scene, nullptr, aiTextureType_BASE_COLOR, dir);
        //  gpuMat.diffuseMap = g_Textures[textureid].bindlessHandle;
    }
}

std::shared_ptr<SceneNode> Scene::parseNode(aiNode* node, const aiScene* scene) {

    auto sceneNode  = std::make_shared<SceneNode>();
    sceneNode->name = node->mName.C_Str();
    CORE_INFO("{}", node->mName.C_Str());

    // Row-major matrix translation from Assimp to GLM
    aiMatrix4x4 t = node->mTransformation;
    sceneNode->localTransform =
        glm::mat4(t.a1, t.b1, t.c1, t.d1, t.a2, t.b2, t.c2, t.d2, t.a3, t.b3, t.c3, t.d3, t.a4, t.b4, t.c4, t.d4);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        sceneNode->meshes.push_back(processMesh(mesh));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        sceneNode->children.push_back(parseNode(node->mChildren[i], scene));
        sceneNode->children[i]->parent = sceneNode.get();
    }

    return sceneNode;
}

Mesh Scene::processMesh(aiMesh* mesh) {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal =
            mesh->HasNormals() ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : glm::vec3(0.f);
        vertex.texCoords =
            mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.f);
        vertex.tangent = mesh->HasTangentsAndBitangents()
                             ? glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f)
                             : glm::vec4(0.f);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    Mesh glMesh;
    glMesh.setup(vertices, indices, mesh->mMaterialIndex);

    return glMesh;
}

bool Scene::LoadGltf(const std::filesystem::path& path) {
    Assimp::Importer importer;

    // Use standard postprocessing configuration options to sanitize incoming
    // graphics structures
    static constexpr unsigned int IMPORT_FLAGS =
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_GlobalScale;

    const aiScene* scene = importer.ReadFile(path.string(), IMPORT_FLAGS);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        CORE_ERROR("[Assimp Error]: {}", importer.GetErrorString());
        return false;
    }

    Clear();
    m_RootNodes.push_back(parseNode(scene->mRootNode, scene));
    loader::assimp::ProcaessMaterials(scene, path.parent_path().string());
    Update();
    return true;
}

} // namespace lgt