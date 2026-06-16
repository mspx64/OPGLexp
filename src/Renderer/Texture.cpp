#include "Texture.h"
#include "helpers/Logger.h"

namespace lgt {

std::unordered_map<std::string, Texture>                   g_Textures;
std::unordered_map<SamplerState, GLuint, SamplerStateHash> g_SamplerCache;

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
    texture.width  = desc.width;
    texture.height = desc.height;

    // this is intended  to use as the validation falg in the engine
    // TODO add more roboust validation
    texture.isValid = true;

    if (desc.generateMips)
        levels = static_cast<GLsizei>(std::floor(std::log2(std::max<float>(desc.width, desc.height)))) + 1;

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

    glDeleteTextures(1, &it->second.handle);
    g_Textures.erase(it);
}

GLuint CreateSampler(const SamplerState& state) {
    GLuint sampler;
    glCreateSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, state.wrapMode);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, state.wrapMode);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, state.magFilter);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, state.minFilter);

    return sampler;
    // TODO add support for the max anistr...
}

GLuint64 GetBindlessTextureSamplerHandle(GLuint textureHandle, SamplerState sampler) {

    auto     it = g_SamplerCache.find(sampler);
    GLuint64 result;
    if (it != g_SamplerCache.end()) {
        // TODO make the cache for the texture bindless handles and query the cache first before calling the dirver
        result = glGetTextureSamplerHandleARB(textureHandle, it->second);
        glMakeTextureHandleResidentARB(result);

    } else {
        auto newSampler = CreateSampler(sampler);
        result          = glGetTextureSamplerHandleARB(textureHandle, newSampler);
        glMakeTextureHandleResidentARB(result);
        // TODO chace the texture-sampler piar ;
        g_SamplerCache[sampler] = newSampler;
    }
    return result;
}

} // namespace lgt