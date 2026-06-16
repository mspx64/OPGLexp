#pragma once

#pragma once
#include <string>
#include <imgui.h>
#include "Renderer/Texture.h"

namespace Editor {

// texture
void DrawTextureSamplerNode(const std::string& textureId, lgt::Texture& texture);

} // namespace Editor