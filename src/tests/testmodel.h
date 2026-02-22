#pragma once

#include "Renderer/renderer.h"
#include "Renderer/Mesh.h"
#include "Renderer/Model.h"
#include "Test.h"
#include "Renderer/Scene.h"


// Forward declarations
class camera;
class ShadowCamera;

// Settings structures for better organization

enum  RenderPassType {
	SHADOW_PASS,
	COLOR_PASS,
};

struct PointLight {
	glm::vec3 direction = glm::vec3(0.0, 0.0, -1.0);
	glm::vec3 position = glm::vec3(2.0f, 4.0f, 2.0f);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
};

struct TransformSettings {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
};

struct PhysicsSettings {
	bool enableGravity = false;
	glm::vec3 velocity = glm::vec3(0.0f);
	float gravityForce = -9.81f;
	float groundLevel = -5.0f;
	float bounceDamping = 0.8f;
};

struct RenderingSettings {
	bool useColor = true;
	glm::vec3 solidColor = glm::vec3(1.0f, 0.0f, 0.0f);
};

struct PerformanceStats {
	float fps = 0.0f;
	float frameTime = 0.0f;
};

class testModel : public Test {

private:
	// Core rendering components
	std::unique_ptr<lgt::Scene> m_scene;
	std::unique_ptr<renderer> m_render;
	std::unique_ptr<Model> m_model;
	std::unique_ptr<Model> m_plane;
	std::unique_ptr<camera> m_camera;
	std::unique_ptr<ShadowCamera> m_shadowcam;
	std::unique_ptr<Grid>   m_grid;

	//Buffers 
	std::unique_ptr<FrameBuffer>m_colorbuffer;
	std::unique_ptr<FrameBuffer>m_shadowdebugbuffer;
	std::unique_ptr<DepthBuffer>m_depthbuffer;

	//shaders
	std::unique_ptr<shader> m_colorshader;
	std::unique_ptr<shader> m_depthshader;
	std::unique_ptr<shader> m_shadowdebugshader;


	// Transformation matrices
	glm::mat4 m_modelMatrix;

	// Settings structures
	PointLight m_lightSettings;
	Material m_materialSettings;
	TransformSettings m_transformSettings;
	PhysicsSettings m_physicsSettings;
	RenderingSettings m_renderingSettings;
	PerformanceStats m_performanceStats;

	// Environment settings
	glm::vec3 m_backgroundColor = glm::vec3(0.1f, 0.1f, 0.15f);
	glm::vec3 m_viewPos = glm::vec3(0.0f, 0.0f, 3.0f);
	ImVec2 m_sceneSize = { 800 , 800 };

	//helpers var
	float m_deltaTime;
	float m_timestep;
	RenderPassType m_renderpasstype = RenderPassType::COLOR_PASS;
	ImGuizmo::OPERATION m_currentop = ImGuizmo::TRANSLATE;

	// Camera and input settings
	float m_speed;
	RenderMode m_renderMode = RenderMode::FILL;

	// Window reference for FPS calculation
	GLFWwindow* m_window = nullptr;

	// Helper methods
	void updateModelMatrix();
	void loadModel(const std::string& filepath);
	void loadShader(const std::string& filepath);

	// ImGui rendering methods

	void createDockSpace();
	void renderMainControlPanel();
	void renderSceneViewport();
	void renderAssetBrowser();
	void renderPerformancePanel();
	void renderLightingTab();
	void renderMaterialTab();
	void renderPhysicsTab();
	void renderEnvironmentTab();
	void renderGizmoControls();
	void setupImGuiStyle();
	void styledSeparator(const char* text);
	void renderPresetControls();
	void renderToolbar();

	//  main render passes 
	void renderColorPass();
	void renderShadowPass();
	void renderShadowDebugPass();

public:
	testModel();
	~testModel();

	void onUpdate(GLFWwindow* window) override;
	void onRender() override;
	void onImguiRender() override;

	// Getters for accessing settings (if needed from outside)
	const PointLight& getLightSettings() const { return m_lightSettings; }
	const Material& getMaterialSettings() const { return m_materialSettings; }
	const TransformSettings& getTransformSettings() const { return m_transformSettings; }
	const RenderPassType& GetRenderPassType()const { return m_renderpasstype; }

	// Setters for programmatic control
	void setLightPosition(const glm::vec3& position) { m_lightSettings.position = position; }
	void setLightColor(const glm::vec3& color) { m_lightSettings.color = color; }
	void setModelPosition(const glm::vec3& position) { m_transformSettings.position = position; }
	void setRenderPass(RenderPassType type) { m_renderpasstype = type; };
};

class Input {


};