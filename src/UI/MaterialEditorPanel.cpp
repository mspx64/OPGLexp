

#include "Renderer/Material.h"
#include "Editor.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace Editor {
void DrawMaterialEditorPanel() {
    ImGui::Begin("Material Editor");

    static std::string selectedMaterialName = "";
    if (lgt::g_MaterialBRDF.empty()) {
        ImGui::Text("No materials loaded in scene.");
        ImGui::End();
        return;
    }

    // Set an initial selection if blank
    if (selectedMaterialName.empty() || lgt::g_MaterialBRDF.find(selectedMaterialName) == lgt::g_MaterialBRDF.end()) {
        selectedMaterialName = lgt::g_MaterialBRDF.begin()->first;
    }

    if (ImGui::BeginCombo("Active Material", selectedMaterialName.c_str())) {
        for (const auto& [name, brdf] : lgt::g_MaterialBRDF) {
            bool        isSelected  = (selectedMaterialName == name);
            std::string displayName = name.empty() ? "Unnamed Material" : name;
            std::string uniqueLabel = displayName + "##mat_" + std::to_string(brdf.gpuIndex);
            if (ImGui::Selectable(uniqueLabel.c_str(), isSelected)) {
                selectedMaterialName = name;
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
        if (ImGui::ColorEdit4("Emissive Color", &currentGPUData.emmisiveCOlor.x))
            trackChanges = true;
        if (ImGui::ColorEdit4("Specular Color", &currentGPUData.specularColor.x))
            trackChanges = true;
        if (ImGui::ColorEdit4("Diffuse Color", &currentGPUData.diffuseColor.x))
            trackChanges = true;

        ImGui::Spacing();
        if (ImGui::SliderFloat("Roughness", &currentGPUData.roughness, 0.0f, 1.0f))
            trackChanges = true;
        if (ImGui::SliderFloat("Metalic", &currentGPUData.metalic, 0.0f, 1.0f))
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

        displayTextureSlot("Base Color Map", currentGPUData.baseColorMap);
        displayTextureSlot("Normal Map", currentGPUData.normalMap);
        displayTextureSlot("Metallic Roughness", currentGPUData.metallicRoughnessMap);
    }

    // 5. Per-Texture Sampler Modifiers
    if (ImGui::CollapsingHeader("Texture Sampler")) {
        for (size_t i = 0; i < activeBRDF.textures.size(); ++i) {
            auto& tex = activeBRDF.textures[i];

            std::string headerName = "Slot [" + std::to_string(i) + "] - ID: " + tex.textureId;
            if (ImGui::TreeNode(headerName.c_str())) {

                ImGui::Text("Texture Handle: %u", tex.textureHandle);

                // Read-only sampler lookups for your debugging sanity
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
        // Push update notification down to your engine implementation
        lgt::UpateMaterial(gpuIndex, currentGPUData);
    }

    ImGui::End();
}
} // namespace Editor
