#pragma once
#include "cstdint"
#include <iostream>
#include <memory>

#include "glad.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For transformations like translate, rotate, perspective
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "BufferLayout.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "shader.h"
#include "Material.h"

#define SHADOW_WIDTH  2048
#define SHADOW_HEIGHT 2048

namespace lgt {
class Camera;
class Renderer;
class Scene;
struct SceneNode;
using RenderId = unsigned int;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec4 tangent;
};

struct Mesh {
    GLuint   vao, vbo, ibo;
    uint32_t indexCount;
    uint32_t materialIndex;

    void setup(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, uint32_t matIdx) {
        materialIndex = matIdx;
        indexCount    = indices.size();

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // TexCoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        // Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        glBindVertexArray(0);
    }
};

struct GridSettings {
    glm::vec3 baseColor       = glm::vec3(0.1f, 0.1f, 0.15f);
    glm::vec3 gradientColor   = glm::vec3(0.2f, 0.4f, 0.8f);
    float     fadeDistance    = 50.0f;
    float     gridIntensity   = 0.5f;
    float     waveAmplitude   = 0.5f;
    float     waveFrequency   = 0.1f;
    bool      enableAnimation = false;
    bool      enableGrid      = true;
    bool      enableGradient  = true;
};

class FrameBuffer {
private:
    float    m_width, m_height;
    RenderId m_FBO;
    RenderId m_textureId;
    RenderId m_renderbuffer;
    friend class Renderer;

public:
    void Use();
    void Unuse();

    RenderId GetTextureId();
    RenderId GetHeight();
    RenderId GetWidth();
    void     Resize(int w, int h);

    FrameBuffer(float w, float h);
    ~FrameBuffer();
};

class DepthBuffer {
private:
    RenderId m_BufferId;
    RenderId m_textureId;

public:
    DepthBuffer();
    ~DepthBuffer();
    RenderId GetTextureId();

    void BindTex(unsigned int unit);
    void UnBindTex();
    void Use();
    void Unsue();
    void Bind();
    void UnBind();
};

class Grid {

private:
    std::unique_ptr<Pipeline> m_gridShader;
    GLuint                    m_VAO, m_VBO, m_EBO;
    std::vector<float>        m_vertices;
    std::vector<unsigned int> m_indices;

    // Animation settings
    GridSettings m_settings;

    float m_time = 0.0f;

public:
    Grid();
    ~Grid();
    void          generateGridMesh(int width, int height, float spacing);
    void          setupBuffers();
    void          render(Camera& cam, float deltaTime);
    GridSettings& getSetting();
    void          cleanup();
};

enum class RenderMode {
    FILL,
    WIREFRAME,
    POINT
};

// OpenGL debug macro
#define GlCall(x)                                                                                                                \
    lgt::Renderer::GLClearError();                                                                                               \
    x;                                                                                                                           \
    if (!lgt::Renderer::GLLogCall(#x, __FILE__, __LINE__))                                                                       \
        __debugbreak();

class Renderer {
public:
    Renderer(Scene* scene, Camera* camera);

    ~Renderer();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&&)            = default;
    Renderer& operator=(Renderer&&) = default;

    void init();
    void shutdown();

    static void GLClearError();
    static bool GLLogCall(const char* function, const char* file, int line);

    void Draw(const VertexArray& va, const IndexBuffer& ib, const Pipeline& shaderProgram) const;
    void Clear(const glm::vec3& backgroundColor = glm::vec3(0.0f)) const;

    void Initauad();
    void renderQuad();
    void renderLine(glm::vec3 p1, glm::vec3 p2);
    void renderPoint(glm::vec3 pos);

    float updateAndLogFPS(GLFWwindow* window);
    float getFPS() const { return m_fps; }

    void enableDepthTesting(bool enable = true);
    void enableBlending(bool enable = true);

    // uint32_t addMesh(const VertexBuffer& vb, const IndexBuffer& ib, const
    // Material& mat);
    //  uint32_t addMaterial(const Material& mat);

    RenderMode getRenderMode() const { return m_currentRenderMode; }

    void setRenderMode(RenderMode mode);
    void setViewport(int width, int height);
    void setCamera(Camera* camera);
    void setScene(Scene* Scene);

    // material management
    void createMaterailBuffer(size_t size);
    void uploadMaterialBuffer(MaterialGPU* data, size_t count);
    void updateMaterial(const MaterialGPU& mat, uint32_t index);
    void updateMaterial(const std::string& materialId);

    void updateDirtyRange(MaterialGPU* data, size_t offset, size_t count);

    void passGbuffer();
    void passLighting();

    void render();
    void renderNode(SceneNode* node);

private:
    // inti passes
    void initPassGbuffer();
    void initPassLighitng();

    // Helper methods
    bool               validateDrawCall(const VertexArray& va, const IndexBuffer& ib, const Pipeline& shaderProgram) const;
    static const char* getGLErrorString(GLenum error);

    // Performance monitoring
    mutable int    m_frameCount;
    mutable float  m_fps;
    mutable double m_lastTime;

    RenderId quadVAO = 0;
    RenderId quadVBO;
    RenderId materialSSBO;

    // Render state
    RenderMode m_currentRenderMode;

    Pipeline* testPipeline = nullptr;
    Camera*   camera_      = nullptr;
    Scene*    scene_       = nullptr;
};
} // namespace lgt
