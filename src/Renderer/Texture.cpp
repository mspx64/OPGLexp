#include "Texture.h"
#include "helpers/Logger.h"

namespace lgt {

struct TextureSamplerPairHash {
    std::size_t operator()(const std::pair<GLuint, GLuint>& pair) const {
        std::size_t hash         = 0;
        auto        hash_combine = [&hash](GLuint value) {
            hash ^= std::hash<GLuint>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        };
        hash_combine(pair.first);
        hash_combine(pair.second);
        return hash;
    }
};

struct SamplerStateHash {
    std::size_t operator()(const SamplerState& s) const {
        std::size_t hash         = 0;
        auto        hash_combine = [&hash](GLuint value) {
            hash ^= std::hash<GLuint>{}(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        };
        hash_combine(s.wrapModeS);
        hash_combine(s.wrapModeT);
        hash_combine(s.minFilter);
        hash_combine(s.magFilter);
        return hash;
    }
};

std::unordered_map<std::string, Texture>                                        g_Textures;
std::unordered_map<SamplerState, GLuint, SamplerStateHash>                      g_SamplerCache;
std::unordered_map<std::pair<GLuint, GLuint>, GLuint64, TextureSamplerPairHash> g_BindlessHandleCache;

Texture GetTexture(const std::string& textureId) {
    auto it = g_Textures.find(textureId);
    if (it != g_Textures.end())
        return it->second;
    CORE_ERROR("Texture not found {}", textureId);
    return Texture{};
}

Texture CreateTexture(const TextureDesc& desc) {
    GLenum  internalFormat = GL_RGBA8;
    GLenum  format         = GL_RGBA;
    GLsizei levels         = 1;

    Texture texture{};
    texture.width   = desc.width;
    texture.height  = desc.height;
    texture.isValid = true;

    if (desc.generateMips)
        levels = static_cast<GLsizei>(std::floor(std::log2(static_cast<float>(std::max(desc.width, desc.height))))) + 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture.handle);
    glTextureStorage2D(texture.handle, levels, internalFormat, texture.width, texture.height);
    glTextureSubImage2D(texture.handle, 0, 0, 0, texture.width, texture.height, format, GL_UNSIGNED_BYTE, desc.data);

    if (desc.generateMips)
        glGenerateTextureMipmap(texture.handle);

    return texture;
}

void DestroyTexture(const std::string& textureId) {
    auto it = g_Textures.find(textureId);
    if (it == g_Textures.end())
        return;

    // TODO Clean up: In a full engine, any bindless handles
    // associated with this texture.handle, call glMakeTextureHandleNonResidentARB,
    // and remove them from g_BindlessHandleCache before deleting the texture.

    glDeleteTextures(1, &it->second.handle);
    g_Textures.erase(it);
}

GLuint CreateSampler(const SamplerState& state) {
    GLuint sampler;
    glCreateSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, state.wrapModeS);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, state.wrapModeT);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, state.magFilter);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, state.minFilter);

    // Optional: Add anisotropic filtering support here later if desired!
    return sampler;
}

GLuint64 GetBindlessTextureSamplerHandle(GLuint textureHandle, SamplerState sampler) {

    GLuint samplerID = 0;
    auto   samplerIt = g_SamplerCache.find(sampler);

    if (samplerIt != g_SamplerCache.end()) {
        samplerID = samplerIt->second;
    } else {
        samplerID               = CreateSampler(sampler);
        g_SamplerCache[sampler] = samplerID;
    }

    std::pair<GLuint, GLuint> cacheKey   = {textureHandle, samplerID};
    auto                      bindlessIt = g_BindlessHandleCache.find(cacheKey);

    if (bindlessIt != g_BindlessHandleCache.end()) {
        return bindlessIt->second;
    }

    GLuint64 bindlessHandle = glGetTextureSamplerHandleARB(textureHandle, samplerID);
    glMakeTextureHandleResidentARB(bindlessHandle);

    g_BindlessHandleCache[cacheKey] = bindlessHandle;

    return bindlessHandle;
}

} // namespace lgt