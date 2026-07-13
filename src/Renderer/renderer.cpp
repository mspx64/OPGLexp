#include "Renderer.h"
#include "helpers/Logger.h"
#include "Scene.h"
#include "Camera.h"
#include <chrono>
#include <iostream>

//----------------------------------------------------------------------
namespace lgt {
Grid::Grid() {
    m_gridShader = std::make_unique<Pipeline>("res/shaders/grid.shader");

    generateGridMesh(100, 100, 20.0f); // 100x100 grid, 20 units spacing
    setupBuffers();
}

Grid::~Grid() {
    cleanup();
}

void Grid::generateGridMesh(int width, int height, float spacing) {
    m_vertices.clear();
    m_indices.clear();

    // Generate vertices
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float xPos = (x - width * 0.5f) * spacing;
            float zPos = (z - height * 0.5f) * spacing;

            // Position (y = 0 for flat grid, will be displaced in vertex shader)
            m_vertices.push_back(xPos); // x
            m_vertices.push_back(0.0f); // y
            m_vertices.push_back(zPos); // z
        }
    }

    // Generate indices for wireframe or triangles
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft     = z * width + x;
            int topRight    = topLeft + 1;
            int bottomLeft  = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // Create triangles
            m_indices.push_back(topLeft);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(topRight);

            m_indices.push_back(topRight);
            m_indices.push_back(bottomLeft);
            m_indices.push_back(bottomRight);
        }
    }
}

void Grid::setupBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Grid::render(Camera& cam, float deltaTime) {
    if (!m_gridShader || !m_gridShader->isValid())
        return;

    m_time += deltaTime;

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth writing but keep depth testing
    glDepthMask(GL_FALSE);

    // Use RAII shader binding
    m_gridShader->use();
    // Set matrices
    glm::mat4 model = glm::mat4(1.0f);
    m_gridShader->setMat4("u_model", model);
    m_gridShader->setMat4("u_view", cam.GetViewMatrix());
    m_gridShader->setMat4("u_projection", cam.GetProjectionMatrix());

    // Set view position
    m_gridShader->setVec3("u_viewPos", cam.GetCameraPos());

    // Set animation parameters
    m_gridShader->setFloat("u_time", m_time);
    m_gridShader->setBool("u_enableAnimation", m_settings.enableAnimation);
    m_gridShader->setFloat("u_waveAmplitude", m_settings.waveAmplitude);
    m_gridShader->setFloat("u_waveFrequency", m_settings.waveFrequency);

    // Set appearance parameters
    m_gridShader->setVec3("u_baseColor", m_settings.baseColor);
    m_gridShader->setVec3("u_gradientColor", m_settings.gradientColor);
    m_gridShader->setFloat("u_fadeDistance", m_settings.fadeDistance);
    m_gridShader->setFloat("u_gridIntensity", m_settings.gridIntensity);
    m_gridShader->setBool("u_enableGrid", m_settings.enableGrid);
    m_gridShader->setBool("u_enableGradient", m_settings.enableGradient);

    // Render the mesh
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Restore render state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

GridSettings& Grid::getSetting() {
    return m_settings;
}

void Grid::cleanup() {
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        m_VAO = m_VBO = m_EBO = 0;
    }
}

Renderer::Renderer(Scene* scene, Camera* camera)
    : m_frameCount(0),
      m_fps(0.0f),
      m_lastTime(0.0),
      m_currentRenderMode(RenderMode::FILL) {

    ASSERT(scene && camera);
    camera_ = camera;
    scene_  = scene;
}

Renderer::~Renderer() {}

void Renderer::init() {
    testPipeline = new Pipeline("res/shaders/PBR.shader");
    createMaterailBuffer(g_MaterialGPU.size());
    uploadMaterialBuffer(g_MaterialGPU.data(), g_MaterialGPU.size());
}

void Renderer::shutdown() {
    delete testPipeline;
}

void Renderer::GLClearError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool Renderer::GLLogCall(const char* function, const char* file, int line) {
    bool hasError = false;

    while (GLenum error = glGetError()) {
        const char* errorStr = getGLErrorString(error);
        CORE_CRITICAL("OPEGL : {} ", errorStr);
        hasError = true;
    }

    return !hasError;
}

const char* Renderer::getGLErrorString(GLenum error) {
    switch (error) {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    default:
        return "Unknown GL Error";
    }
}

void Renderer::Clear(const glm::vec3& backgroundColor) const {
    GlCall(glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f));
    GlCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

float Renderer::updateAndLogFPS(GLFWwindow* window) {
    const double currentTime = glfwGetTime();
    const double deltaTime   = currentTime - m_lastTime;

    ++m_frameCount;

    if (deltaTime >= 0.016) {
        m_fps        = static_cast<float>(m_frameCount / deltaTime);
        m_lastTime   = currentTime;
        m_frameCount = 0;
    }

    return m_fps;
}

void Renderer::setRenderMode(RenderMode mode) {
    if (m_currentRenderMode == mode) {
        return; // Avoid unnecessary state changes
    }

    m_currentRenderMode = mode;

    switch (mode) {
    case RenderMode::FILL:
        GlCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        break;
    case RenderMode::WIREFRAME:
        GlCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        break;
    case RenderMode::POINT:
        GlCall(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));
        GlCall(glPointSize(1.0f));
        break;
    }
}

void Renderer::enableDepthTesting(bool enable) {
    if (enable) {
        GlCall(glEnable(GL_DEPTH_TEST));
    } else {
        GlCall(glDisable(GL_DEPTH_TEST));
    }
}

void Renderer::enableBlending(bool enable) {
    if (enable) {
        GlCall(glEnable(GL_BLEND));
        GlCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    } else {
        GlCall(glDisable(GL_BLEND));
    }
}

void Renderer::setViewport(int width, int height) {
    if (width > 0 && height > 0) {
        GlCall(glViewport(0, 0, width, height));
    }
}

void Renderer::setCamera(Camera* camera) {
    if (!camera)
        return;
    camera_ = camera;
}

void Renderer::setScene(Scene* Scene) {
    if (!Scene)
        return;
    scene_ = Scene;
}

void Renderer::setDebugMode(DebugMode mode) {
    ASSERT(testPipeline);
    testPipeline->use();
    testPipeline->setInt("u_DebugMode", (int)mode);
}

void Renderer::createMaterailBuffer(size_t size) {
    if (size == 0) size = 1; // Prevent GL_INVALID_VALUE on empty scenes
    
    if (g_MaterialSSBO != 0) {
        glDeleteBuffers(1, &g_MaterialSSBO);
        g_MaterialSSBO = 0;
    }
    
    glCreateBuffers(1, &g_MaterialSSBO);
    glNamedBufferStorage(g_MaterialSSBO, size * sizeof(MaterialGPU), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_MaterialSSBO);
}

void Renderer::uploadMaterialBuffer(MaterialGPU* data, size_t size) {
    glNamedBufferSubData(g_MaterialSSBO, 0, size * sizeof(MaterialGPU), data);
}

void Renderer::updateMaterial(const MaterialGPU& mat, uint32_t index) {
    glNamedBufferSubData(g_MaterialSSBO, index * sizeof(MaterialGPU), sizeof(MaterialGPU), &mat);
}

void Renderer::updateDirtyRange(MaterialGPU* data, size_t offset, size_t count) {
    glNamedBufferSubData(g_MaterialSSBO, offset * sizeof(MaterialGPU), sizeof(MaterialGPU), data);
}

void Renderer::Initauad() {
    if (quadVAO != 0)
        return;

    float quadVertices[] = {// positions   // texcoords
                            -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void Renderer::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

//-----------------------FRAMEBUFFER--------------------------

RenderId FrameBuffer::GetTextureId() {
    return m_textureId;
}

RenderId FrameBuffer::GetHeight() {
    return static_cast<RenderId>(m_height);
}

RenderId FrameBuffer::GetWidth() {
    return static_cast<RenderId>(m_width);
}
void FrameBuffer::Resize(int w, int h) {
    m_width  = w;
    m_height = h;
    GlCall(glBindTexture(GL_TEXTURE_2D, m_textureId));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GlCall(glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer));
    GlCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height));
}
//
void FrameBuffer::Use() {
    GlCall(glViewport(0, 0, m_width, m_height));
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
}

void FrameBuffer::Unuse() {
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

FrameBuffer::FrameBuffer(int w, int h)
    : m_width(w),
      m_height(h) {
    GlCall(glGenFramebuffers(1, &m_FBO));
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));

    GlCall(glGenTextures(1, &m_textureId));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_textureId));
    GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GlCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureId, 0));

    GlCall(glGenRenderbuffers(1, &m_renderbuffer));
    GlCall(glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer));

    GlCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height));
    GlCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        CORE_ERROR("FRAMEBUFFER Not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteRenderbuffers(1, &m_renderbuffer);
    glDeleteTextures(1, &m_textureId);
}

//-------------------------------------------------------

DepthBuffer::DepthBuffer() {
    GlCall(glGenFramebuffers(1, &m_BufferId));
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_BufferId));

    // Generate depth texture
    GlCall(glGenTextures(1, &m_textureId));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_textureId));
    GlCall(glTexImage2D(GL_TEXTURE_2D,
                        0,
                        GL_DEPTH_COMPONENT16,
                        (GLsizei)SHADOW_WIDTH,
                        (GLsizei)SHADOW_HEIGHT,
                        0,
                        GL_DEPTH_COMPONENT,
                        GL_FLOAT,
                        nullptr));

    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    GlCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureId, 0));

    GlCall(glDrawBuffer(GL_NONE));
    GlCall(glReadBuffer(GL_NONE));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Depth framebuffer not complete!" << std::endl;
    }

    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

DepthBuffer::~DepthBuffer() {
    GlCall(glDeleteFramebuffers(1, &m_BufferId));
    GlCall(glDeleteTextures(1, &m_textureId));
}

void DepthBuffer::Use() {
    GlCall(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_BufferId));
}
void DepthBuffer::Unsue() {
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void DepthBuffer::Bind() {
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, m_BufferId));
}
void DepthBuffer::UnBind() {
    GlCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void DepthBuffer::BindTex(unsigned int slot) {
    GlCall(glActiveTexture(GL_TEXTURE0 + slot));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_textureId));
}

void DepthBuffer::UnBindTex() {
    GlCall(glBindTexture(GL_TEXTURE_2D, 0));
}

RenderId DepthBuffer::GetTextureId() {
    return m_textureId;
}

void Renderer::render() {
    if (!scene_) {
        CORE_ERROR("No scene available");
        return;
    }
    if (!testPipeline) {
        CORE_ERROR("No pipeline available");
        return;
    }

    // Dynamically rebuild and upload the Material SSBO if new models/materials were loaded
    static size_t lastMaterialCount = 0;
    if (g_MaterialGPU.size() != lastMaterialCount) {
        lastMaterialCount = g_MaterialGPU.size();
        createMaterailBuffer(lastMaterialCount);
        if (lastMaterialCount > 0) {
            uploadMaterialBuffer(g_MaterialGPU.data(), lastMaterialCount);
        }
    }

    testPipeline->useWithCamera(*camera_);

    for (auto sceneNode : scene_->getRootNodes()) {
        renderNode(sceneNode.get());
    }
}

void Renderer::renderNode(SceneNode* node) {
    testPipeline->setMat4("u_Model", node->globalTransform);

    for (auto& mesh : node->meshes) {
        testPipeline->setMaterial(mesh.materialIndex);
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, 0);
    }
    for (auto child : node->children) {
        renderNode(child.get());
    }
}

} // namespace lgt
