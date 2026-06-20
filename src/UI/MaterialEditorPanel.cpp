#include "Renderer/Material.h"
#include "Editor.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace Editor {
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