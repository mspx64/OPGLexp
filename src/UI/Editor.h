#pragma once

#pragma once
#include <string>
#include <imgui.h>
#include "Renderer/Texture.h"

namespace lgt {
class Renderer;
class Scene;
}

namespace Editor {
void ApplyProfessionalTheme();
void DrawPerformancePanel(lgt::Renderer* renderer);
void DrawSceneHierarchyPanel(lgt::Scene* scene);

void DrawMaterialEditorPanel();
// texture
void DrawTextureSamplerNode(const std::string& textureId, lgt::Texture& texture);

} // namespace Editor