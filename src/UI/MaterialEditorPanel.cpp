#include "Renderer/Material.h"
#include "Renderer/Renderer.h"
#include "Renderer/Scene.h"
#include "Renderer/Camera.h"
#include "Renderer/Texture.h"
#include "Editor.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

using namespace lgt;

static lgt::SceneNode* g_SelectedNode = nullptr;

namespace Editor {

void ApplyProfessionalTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Professional rounded edges
    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    // Clean borders
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.16f, 0.51f, 0.86f, 1.00f); // Blue accent
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.61f, 0.96f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.16f, 0.51f, 0.86f, 0.80f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.16f, 0.51f, 0.86f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.16f, 0.51f, 0.86f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.16f, 0.51f, 0.86f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.16f, 0.51f, 0.86f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.16f, 0.51f, 0.86f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.16f, 0.51f, 0.86f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.07f, 0.07f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.16f, 0.51f, 0.86f, 0.35f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.16f, 0.51f, 0.86f, 1.00f);
}

static SceneNode* s_NodeToDelete = nullptr;

static void DrawSceneNodeRecursive(lgt::SceneNode* node) {
    if (!node) return;
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node->children.empty() && node->meshes.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    
    std::string displayName = node->name.empty() ? "Unnamed Node" : node->name;
    
    if (node == g_SelectedNode) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    bool nodeOpen = ImGui::TreeNodeEx((void*)node, flags, "%s", displayName.c_str());
    
    if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
        g_SelectedNode = node;
    }
    
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete")) {
            s_NodeToDelete = node;
            if (g_SelectedNode == node) g_SelectedNode = nullptr;
        }
        ImGui::EndPopup();
    }
    
    if (nodeOpen && !(flags & ImGuiTreeNodeFlags_Leaf)) {
        // Render child meshes
        for (size_t i = 0; i < node->meshes.size(); ++i) {
            ImGui::TreeNodeEx((void*)&node->meshes[i], ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "Mesh %zu", i);
        }
        // Render child nodes
        for (auto& child : node->children) {
            DrawSceneNodeRecursive(child.get());
        }
        ImGui::TreePop();
    }
}

void DrawSceneHierarchyPanel(lgt::Scene* scene) {
    if (!scene) return;
    
    ImGui::Begin("Scene Hierarchy");
    
    s_NodeToDelete = nullptr;
    auto& roots = scene->getRootNodes();
    for (auto& root : roots) {
        DrawSceneNodeRecursive(root.get());
    }
    
    if (s_NodeToDelete) {
        scene->RemoveNode(s_NodeToDelete);
        s_NodeToDelete = nullptr;
    }
    
    ImGui::End();
}



void DrawEnvironmentPanel(lgt::Grid* grid, lgt::Renderer* renderer) {
    ImGui::Begin("Environment & Renderer");
    
    if (ImGui::CollapsingHeader("Renderer Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (renderer) {
            int currentMode = (int)renderer->getRenderMode();
            const char* items[] = { "Fill", "Wireframe", "Point" };
            if (ImGui::Combo("Render Mode", &currentMode, items, IM_ARRAYSIZE(items))) {
                renderer->setRenderMode((lgt::RenderMode)currentMode);
            }
        }
    }
    
    if (ImGui::CollapsingHeader("Grid Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (grid) {
            lgt::GridSettings& settings = grid->getSetting();
            ImGui::Checkbox("Enable Grid", &settings.enableGrid);
            if (settings.enableGrid) {
                ImGui::ColorEdit3("Base Color", &settings.baseColor.x);
                ImGui::Checkbox("Enable Gradient", &settings.enableGradient);
                if (settings.enableGradient) {
                    ImGui::ColorEdit3("Gradient Color", &settings.gradientColor.x);
                }
                ImGui::SliderFloat("Fade Distance", &settings.fadeDistance, 10.0f, 200.0f);
                ImGui::SliderFloat("Grid Intensity", &settings.gridIntensity, 0.0f, 1.0f);
                
                ImGui::Checkbox("Enable Animation", &settings.enableAnimation);
                if (settings.enableAnimation) {
                    ImGui::SliderFloat("Wave Amplitude", &settings.waveAmplitude, 0.0f, 2.0f);
                    ImGui::SliderFloat("Wave Frequency", &settings.waveFrequency, 0.0f, 2.0f);
                }
            }
        }
    }
    
    ImGui::End();
}

void DrawCameraPanel(lgt::Camera* camera, float* speed, float* sensitivity) {
    if (!camera) return;
    
    ImGui::Begin("Camera Controls");
    
    if (ImGui::CollapsingHeader("Movement Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (speed) ImGui::SliderFloat("Move Speed", speed, 0.0001f, 0.01f, "%.4f");
        if (sensitivity) ImGui::SliderFloat("Sensitivity", sensitivity, 1.0f, 100.0f);
    }
    
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 pos = camera->GetCameraPos();
        glm::vec3 dir = camera->GetDirection();
        
        ImGui::Text("Position: X: %.2f  Y: %.2f  Z: %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("Direction: X: %.2f  Y: %.2f  Z: %.2f", dir.x, dir.y, dir.z);
    }
    
    ImGui::End();
}

void DrawAssetBrowserPanel(lgt::Scene* scene) {
    if (!scene) return;
    
    ImGui::Begin("Asset Browser");
    
    std::filesystem::path modelsPath = "res/modles"; // Correcting typo locally if needed, but sticking to existing structure
    if (!std::filesystem::exists(modelsPath)) {
        modelsPath = "res/models"; // Fallback to correct spelling just in case
    }
    
    if (std::filesystem::exists(modelsPath)) {
        static GLuint s_GltfIcon = 0;
        static GLuint s_FbxIcon = 0;
        
        if (s_GltfIcon == 0) {
            int width, height, nrChannels;
            unsigned char* data = stbi_load("res/images/icon_gltf.jpg", &width, &height, &nrChannels, 4);
            if (data) {
                glGenTextures(1, &s_GltfIcon);
                glBindTexture(GL_TEXTURE_2D, s_GltfIcon);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                stbi_image_free(data);
            }
        }
        
        if (s_FbxIcon == 0) {
            int width, height, nrChannels;
            unsigned char* data = stbi_load("res/images/icon_fbx.jpg", &width, &height, &nrChannels, 4);
            if (data) {
                glGenTextures(1, &s_FbxIcon);
                glBindTexture(GL_TEXTURE_2D, s_FbxIcon);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                stbi_image_free(data);
            }
        }
        
        static float padding = 16.0f;
        static float thumbnailSize = 90.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        if (ImGui::BeginTable("AssetGrid", columnCount)) {
            for (auto& entry : std::filesystem::recursive_directory_iterator(modelsPath)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".gltf" || ext == ".glb" || ext == ".fbx") {
                        ImGui::TableNextColumn();
                        std::string filename = entry.path().filename().string();
                        
                        std::string filepath = entry.path().string();
                        ImGui::PushID(filepath.c_str());
                        
                        // Push button colors to look more like a Unity thumbnail backing
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                        
                        bool clicked = false;
                        
                        GLuint activeIcon = 0;
                        if (ext == ".gltf" || ext == ".glb") activeIcon = s_GltfIcon;
                        else if (ext == ".fbx") activeIcon = s_FbxIcon;
                        
                        if (activeIcon != 0) {
                            clicked = ImGui::ImageButton(filename.c_str(), (ImTextureID)(intptr_t)activeIcon, ImVec2(thumbnailSize, thumbnailSize));
                        } else {
                            clicked = ImGui::Button("MODEL", ImVec2(thumbnailSize, thumbnailSize));
                        }
                        
                        if (clicked) {
                            scene->LoadGltf(entry.path().string());
                        }
                        ImGui::PopStyleColor(3);
                        
                        // Attempt to center text under the button
                        float textWidth = ImGui::CalcTextSize(filename.c_str()).x;
                        float offset = (thumbnailSize - textWidth) * 0.5f;
                        if (offset > 0.0f) {
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
                        }
                        ImGui::TextWrapped("%s", filename.c_str());
                        ImGui::PopID();
                    }
                }
            }
            ImGui::EndTable();
        }
        
    } else {
        ImGui::Text("Models directory not found.");
    }
    
    ImGui::End();
}

void DrawInspectorPanel() {
    ImGui::Begin("Inspector");
    
    static lgt::SceneNode* s_LastSelectedNode = nullptr;
    static glm::vec3 s_Translation(0.0f);
    static glm::vec3 s_RotationEuler(0.0f);
    static glm::vec3 s_Scale(1.0f);

    if (g_SelectedNode) {
        if (g_SelectedNode != s_LastSelectedNode) {
            s_LastSelectedNode = g_SelectedNode;
            
            // Decompose the localTransform matrix only once upon selection
            glm::quat rotationQuat;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(g_SelectedNode->localTransform, s_Scale, rotationQuat, s_Translation, skew, perspective);
            
            // Convert quaternion to Euler angles (in degrees for UI)
            s_RotationEuler = glm::degrees(glm::eulerAngles(rotationQuat));
        }

        ImGui::Text("Selected: %s", g_SelectedNode->name.empty() ? "Unnamed Node" : g_SelectedNode->name.c_str());
        ImGui::Separator();
        
        bool modified = false;
        
        if (ImGui::DragFloat3("Translation", glm::value_ptr(s_Translation), 0.1f)) modified = true;
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(s_RotationEuler), 1.0f)) modified = true;
        if (ImGui::DragFloat3("Scale", glm::value_ptr(s_Scale), 0.1f)) modified = true;
        
        if (modified) {
            // Reconstruct the transformation matrix from our cached TRS components
            glm::quat newRot = glm::quat(glm::radians(s_RotationEuler));
            glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), s_Translation) * 
                                     glm::mat4_cast(newRot) * 
                                     glm::scale(glm::mat4(1.0f), s_Scale);
            
            g_SelectedNode->localTransform = newTransform;
            
            // Recalculate global transforms down the hierarchy
            g_SelectedNode->UpdateTransformCascades();
        }
    } else {
        s_LastSelectedNode = nullptr;
        ImGui::Text("No node selected.");
    }
    
    ImGui::End();
}

void DrawMaterialEditorPanel() {
    ImGui::Begin("Material Editor");

    if (lgt::g_MaterialBRDF.empty()) {
        ImGui::Text("No materials loaded in scene.");
        ImGui::End();
        return;
    }

    static uint32_t selectedGpuIndex = 0xFFFFFFFF;
    static bool     isInitialized    = false;

    if (!isInitialized) {
        selectedGpuIndex = lgt::g_MaterialBRDF.begin()->second.gpuIndex;
        isInitialized    = true;
    }

    std::string selectedMaterialName = "";
    bool        foundActive          = false;
    for (const auto& [name, brdf] : lgt::g_MaterialBRDF) {
        if (brdf.gpuIndex == selectedGpuIndex) {
            selectedMaterialName = name;
            foundActive          = true;
            break;
        }
    }

    if (!foundActive) {
        selectedGpuIndex     = lgt::g_MaterialBRDF.begin()->second.gpuIndex;
        selectedMaterialName = lgt::g_MaterialBRDF.begin()->first;
    }

    std::string comboLabel = selectedMaterialName.empty() ? "Unnamed Material" : selectedMaterialName;
    if (ImGui::BeginCombo("Active Material", comboLabel.c_str())) {
        for (const auto& [name, brdf] : lgt::g_MaterialBRDF) {
            bool        isSelected  = (brdf.gpuIndex == selectedGpuIndex);
            std::string displayName = name.empty() ? "Unnamed Material" : name;
            std::string uniqueLabel = displayName + "##mat_" + std::to_string(brdf.gpuIndex);

            if (ImGui::Selectable(uniqueLabel.c_str(), isSelected)) {
                selectedGpuIndex = brdf.gpuIndex;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    auto&    activeBRDF = lgt::g_MaterialBRDF[selectedMaterialName];
    uint32_t gpuIndex   = activeBRDF.gpuIndex;

    lgt::MaterialGPU currentGPUData = lgt::g_MaterialGPU[gpuIndex];
    bool             trackChanges   = false;

    ImGui::TextDisabled("GPU Buffer Index: %u", gpuIndex);

    if (ImGui::CollapsingHeader("PBR Colors & Factors", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::ColorEdit4("Base Color", &currentGPUData.baseColor.x))
            trackChanges = true;
        if (ImGui::ColorEdit4("Emissive Color", &currentGPUData.emmisiveColor.x))
            trackChanges = true;

        ImGui::Spacing();
        if (ImGui::SliderFloat("Roughness", &currentGPUData.roughness, 0.0f, 1.0f))
            trackChanges = true;
        if (ImGui::SliderFloat("Metallic", &currentGPUData.metallic, 0.0f, 1.0f))
            trackChanges = true;
        if (ImGui::SliderFloat("Emissive Strength", &currentGPUData.emmisiveStrength, 0.0f, 20.0f))
            trackChanges = true;
    }

    if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto displayTextureSlot = [](const char* label, GLuint64 handle) {
            ImGui::Text("%s:", label);
            ImGui::SameLine(160);
            if (handle != 0) {
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "0x%llX (Resident)", handle);
            } else {
                ImGui::TextDisabled("None (Fallback Color)");
            }
        };

        displayTextureSlot("Base Color Map", currentGPUData.diffuseMap);
        displayTextureSlot("Normal Map", currentGPUData.normalMap);
        displayTextureSlot("Emissive Map", currentGPUData.emmisiveMap);
    }

    if (ImGui::CollapsingHeader("Texture Sampler")) {
        for (size_t i = 0; i < activeBRDF.textures.size(); ++i) {
            auto& tex = activeBRDF.textures[i];

            std::string headerName = "Slot [" + std::to_string(i) + "] - ID: " + tex.textureId;
            if (ImGui::TreeNode(headerName.c_str())) {

                ImGui::Text("Texture Handle: %u", tex.textureHandle);

                auto getWrapStr = [](GLenum mode) {
                    if (mode == GL_REPEAT)
                        return "REPEAT";
                    if (mode == GL_CLAMP_TO_EDGE)
                        return "CLAMP_TO_EDGE";
                    if (mode == GL_MIRRORED_REPEAT)
                        return "MIRRORED_REPEAT";
                    return "UNKNOWN";
                };

                ImGui::Text("Wrap S (U Axis): %s", getWrapStr(tex.samplerState.wrapModeS));
                ImGui::Text("Wrap T (V Axis): %s", getWrapStr(tex.samplerState.wrapModeT));

                ImGui::TreePop();
                ImGui::Spacing();
            }
        }
    }

    if (trackChanges) {
        lgt::g_MaterialGPU[gpuIndex] = currentGPUData;
        lgt::UpateMaterial(gpuIndex, currentGPUData);
    }

    ImGui::End();
}
} // namespace Editor