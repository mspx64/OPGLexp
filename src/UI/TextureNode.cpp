#include "Editor.h"

namespace Editor {

void DrawTextureSamplerNode(const std::string& textureId, lgt::Texture& texture) {
    ImGui::BeginGroup();

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), " Texture: %s", textureId.c_str());
    ImGui::Separator();
    ImGui::BeginGroup(); // Preview
    if (texture.isValid) {
        ImGui::Image((ImTextureID)(intptr_t)texture.handle, ImVec2(128, 128));
    } else {
        ImGui::Dummy(ImVec2(128, 128));
    }
    ImGui::EndGroup();
    ImGui::SameLine();

    ImGui::EndGroup();
}

} // namespace Editor

// here each material can can reference the same texture differently / with the different sampler state
// so the sampler state must be stored in the material not the texture
//
