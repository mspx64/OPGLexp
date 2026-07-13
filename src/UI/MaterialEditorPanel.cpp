#include "Renderer/Material.h"
#include "Renderer/Renderer.h"
#include "Renderer/Scene.h"
#include "Editor.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>

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

void DrawPerformancePanel(lgt::Renderer* renderer) {
    if (!renderer) return;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    
    // Top-right corner
    window_pos.x = work_pos.x + work_size.x - PAD;
    window_pos.y = work_pos.y + PAD;
    window_pos_pivot.x = 1.0f;
    window_pos_pivot.y = 0.0f;
    
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.7f); // Transparent background

    if (ImGui::Begin("Performance Overlay", nullptr, window_flags)) {
        ImGui::Text("=== SYSTEM METRICS ===");
        ImGui::Separator();
        float fps = renderer->getFPS();
        ImGui::Text("FPS:         %.1f", fps);
        ImGui::Text("Frame Time:  %.3f ms", fps > 0.0f ? 1000.0f / fps : 0.0f);
        ImGui::Separator();
        ImGui::Text("=== GPU INFO ===");
        ImGui::Text("Vendor:      %s", (const char*)glGetString(GL_VENDOR));
        ImGui::Text("Renderer:    %s", (const char*)glGetString(GL_RENDERER));
        ImGui::Separator();
        ImGui::Text("Render Mode: %d", (int)renderer->getRenderMode());
    }
    ImGui::End();
}

static void DrawSceneNodeRecursive(lgt::SceneNode* node) {
    if (!node) return;
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node->children.empty() && node->meshes.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    
    std::string displayName = node->name.empty() ? "Unnamed Node" : node->name;
    bool nodeOpen = ImGui::TreeNodeEx((void*)node, flags, "%s", displayName.c_str());
    
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
    auto& roots = scene->getRootNodes();
    for (auto& root : roots) {
        DrawSceneNodeRecursive(root.get());
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