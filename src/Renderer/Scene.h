#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

// Include Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Renderer.h"

namespace lgt {

struct SceneNode {
    std::string       name;
    std::vector<Mesh> meshes;

    glm::mat4 localTransform  = glm::mat4(1.0f);
    glm::mat4 globalTransform = glm::mat4(1.0f);

    SceneNode*                              parent = nullptr;
    std::vector<std::shared_ptr<SceneNode>> children; // Quick top-down traversal links and strsge

    void UpdateTransformCascades() {
        if (parent) {
            globalTransform = parent->globalTransform * localTransform;
        } else {
            globalTransform = localTransform;
        }

        for (auto child : children) {
            child->UpdateTransformCascades();
        }
    }
};

class Scene {
public:
    Scene()  = default;
    ~Scene() = default;

    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) noexcept            = default;
    Scene& operator=(Scene&&) noexcept = default;

    bool LoadGltf(const std::filesystem::path& path);
    void Update();
    void Clear();

    const std::vector<std::shared_ptr<SceneNode>>& getRootNodes() const { return m_RootNodes; }
    std::vector<MaterialGPU>&                      getMaterialBuffer() { return m_materialBuffer; };

private:
    void                       processMaterials(const aiScene* scene, const std::string& dir);
    std::shared_ptr<SceneNode> parseNode(aiNode* node, const aiScene* scene, const std::vector<uint32_t>& materialMap);
    Mesh                       processMesh(aiMesh* mesh, const std::vector<uint32_t>& materialMap);

    std::vector<MaterialGPU>                m_materialBuffer;
    std::vector<std::shared_ptr<SceneNode>> m_RootNodes;
};

} // namespace lgt