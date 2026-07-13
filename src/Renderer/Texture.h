#pragma once

#include "Vendor/glad.h"
#include "Vendor/stb_image.h"

#include <unordered_map>
#include <string>
#include <cstddef>
#include <functional>

namespace lgt {

enum class TextureType {
    BASE_COLOR,
    NORMAL,
    METALNESS,
    ROUGHNESS,
    AMBIENT_OCCLUSION,
    EMISSIVE
};

struct SamplerState {
    GLenum wrapModeS = GL_REPEAT; // Horizontal axis (U)
    GLenum wrapModeT = GL_REPEAT; // Vertical axis (V)
    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;

    bool operator==(const SamplerState& other) const {
        return wrapModeS == other.wrapModeS && wrapModeT == other.wrapModeT && minFilter == other.minFilter &&
               magFilter == other.magFilter;
    }
};

struct TextureDesc {
    void* data     = nullptr;
    int   channels = 4;
    int   width;
    int   height;
    bool  generateMips = true;

    static TextureDesc Default(int width, int height, void* data) {
        TextureDesc desc{};
        desc.data   = data;
        desc.width  = width;
        desc.height = height;
    }
};

struct Texture {
    int    width, height;
    bool   isValid = false;
    GLuint handle;
};

extern std::unordered_map<std::string, Texture> g_Textures;

Texture  GetTexture(const std::string& textureId);
Texture  CreateTexture(const TextureDesc& desc);
void     DestroyTexture(const std::string& textureId);
GLuint   CreateSampler(const SamplerState& state);
GLuint64 GetBindlessTextureSamplerHandle(GLuint textureHandle, SamplerState);

} // namespace lgt
