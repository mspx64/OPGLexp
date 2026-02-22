#pragma once
#include<iostream>
#include<memory>
#include"cstdint"

#include<GL/glew.h>
#include <GLFW/glfw3.h>

#include<imgui.h>
#include<ImGuizmo.h>
#include<imgui_impl_opengl3.h>
#include<imgui_impl_glfw.h>


#define GLM_ENABLE_EXPERIMENTAL 
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For transformations like translate, rotate, perspective 
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include"Logger.h"
#include"VertexBuffer.h"
#include"IndexBuffer.h"
#include"VertexArray.h"
#include"shader.h"
#include"BufferLayout.h"
#include"Texture.h"

#define SHADOW_WIDTH   2048 
#define SHADOW_HEIGHT  2048

class  camera;
using  RenderId = unsigned int;

struct Material {
	glm::vec3 specular;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	float shininess;
	float normalStrength;
	bool hasNormalMap;
	bool hasSpecularMap;

	// Default constructor
	Material() :
		specular(0.5f),
		ambient(0.2f),
		diffuse(0.8f),
		shininess(100.0f),
		normalStrength(1.0f),
		hasNormalMap(false),
		hasSpecularMap(false) {
	}
};

struct GridSettings {
	glm::vec3 baseColor = glm::vec3(0.1f, 0.1f, 0.15f);
	glm::vec3 gradientColor = glm::vec3(0.2f, 0.4f, 0.8f);
	float fadeDistance = 50.0f;
	float gridIntensity = 0.5f;
	float waveAmplitude = 0.5f;
	float waveFrequency = 0.1f;
	bool  enableAnimation = false;
	bool  enableGrid = true;
	bool  enableGradient = true;
};

class FrameBuffer {
private:

	float m_width, m_height;

	RenderId m_FBO;
	RenderId m_textureId;
	RenderId m_renderbuffer;

public:
	RenderId GetTextureId();
	RenderId GetHeight();
	RenderId GetWidth();
	void Resize(int w, int h);

	void Use();
	void Unuse();
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
	std::unique_ptr<shader> m_gridShader;
	GLuint m_VAO, m_VBO, m_EBO;
	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;

	// Animation settings  
	GridSettings m_settings;

	float m_time = 0.0f;

public:
	Grid();
	~Grid();
	void generateGridMesh(int width, int height, float spacing);
	void setupBuffers();
	void render(camera& cam, float deltaTime);
	GridSettings& getSetting();
	void cleanup();
};

enum class RenderMode {
	FILL,
	WIREFRAME,
	POINT
};

// OpenGL debug macro
#define GlCall(x) renderer::GLClearError(); x; if (!renderer::GLLogCall(#x, __FILE__, __LINE__)) __debugbreak();

class renderer {
public:
	explicit renderer();

	~renderer();

	renderer(const renderer&) = delete;
	renderer& operator=(const renderer&) = delete;

	renderer(renderer&&) = default;
	renderer& operator=(renderer&&) = default;

	static void GLClearError();
	static bool GLLogCall(const char* function, const char* file, int line);

	void Draw(const VertexArray& va, const IndexBuffer& ib, const shader& shaderProgram) const;
	void Clear(const glm::vec3& backgroundColor = glm::vec3(0.0f)) const;

	void Initauad();
	void renderQuad();

	float updateAndLogFPS(GLFWwindow* window);
	float getFPS() const { return m_fps; }

	void logGlVersion() const;

	void setRenderMode(RenderMode mode);
	RenderMode getRenderMode() const { return m_currentRenderMode; }

	void enableDepthTesting(bool enable = true);
	void enableBlending(bool enable = true);
	void setViewport(int width, int height);

private:

	// Helper methods
	bool validateDrawCall(const VertexArray& va, const IndexBuffer& ib, const shader& shaderProgram) const;
	static const char* getGLErrorString(GLenum error);

	// Performance monitoring
	mutable int m_frameCount;
	mutable float m_fps;
	mutable double m_lastTime;

	RenderId quadVAO = 0;
	RenderId quadVBO;
	// Render state
	RenderMode m_currentRenderMode;
};