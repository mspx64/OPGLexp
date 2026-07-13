#pragma once

#pragma once
#include <string>
#include <imgui.h>
#include "Renderer/Texture.h"

namespace lgt {
class Renderer;
class Scene;
class Camera;
class FrameBuffer;
class Grid;
}

namespace Editor {
void ApplyProfessionalTheme();
void DrawSceneHierarchyPanel(lgt::Scene* scene);
void DrawEnvironmentPanel(lgt::Grid* grid, lgt::Renderer* renderer);
void DrawCameraPanel(lgt::Camera* camera, float* speed, float* sensitivity);
void DrawAssetBrowserPanel(lgt::Scene* scene);
void DrawInspectorPanel();
void DrawConsolePanel();

void DrawMaterialEditorPanel();
// texture
void DrawTextureSamplerNode(const std::string& textureId, lgt::Texture& texture);

} // namespace Editor
