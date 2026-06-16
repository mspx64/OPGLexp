#pragma once

#include "glad.h"
#include "stb_image.h"

#include <unordered_map>
#include <string>
#include <cstddef>
#include <functional>

namespace lgt {

enum class TextureType {
    BASE_COLOR,
    NORMAL
};

struct SamplerState {
    GLenum wrapMode  = GL_REPEAT;
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum magFilter = GL_LINEAR;

    bool operator==(const SamplerState& other) const {
        return wrapMode == other.wrapMode && minFilter == other.minFilter && magFilter == other.magFilter;
    }
};

struct SamplerStateHash {
    std::size_t operator()(const SamplerState& s) const {
        std::size_t hash = 0;

        // A standard hash combining magic number (phi) to prevent collisions
        auto hash_combine = [&hash](GLuint value) {
            hash ^= std::hash<GLuint>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        };

        hash_combine(s.wrapMode);
        hash_combine(s.minFilter);
        hash_combine(s.magFilter);

        return hash;
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

extern std::unordered_map<std::string, Texture>                   g_Textures;
extern std::unordered_map<SamplerState, GLuint, SamplerStateHash> g_SamplerCache;

Texture  GetTexture(const std::string& textureId);
Texture  CreateTexture(const TextureDesc& desc);
void     DestroyTexture(const std::string& textureId);
GLuint   CreateSampler(const SamplerState& state);
GLuint64 GetBindlessTextureSamplerHandle(GLuint textureHandle, SamplerState);

} // namespace lgt
