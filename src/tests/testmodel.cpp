#include "testmodel.h"
#include "helpers/Filedial.h"
#include "renderer/camera.h"


testModel::testModel() : m_speed(0.030f)
{
	// ImGui styling
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	setupImGuiStyle();

	// Initialize transformation matrices
	m_modelMatrix = glm::mat4(1.0f);

	// Initialize lighting parameters
	m_lightSettings = {
		glm::vec3(2.0f, 4.0f, 2.0f),  // Direction
		glm::vec3(2.0f, 4.0f, 2.0f),  // position
		glm::vec3(1.0f, 1.0f, 1.0f),  // color
		1.0f,                          // intensity
		1.0f,                          // constant
		0.09f,                         // linear
		0.032f                         // quadratic
	};

	// Initialize rendering components
	m_render = std::make_unique<renderer>();
	m_render->logGlVersion();
	m_render->Initauad();

	// Load default model and shader
	try {
		m_scene = std::make_unique<lgt::Scene>();
		m_model = std::make_unique<Model>("res/modles/Sphere.fbx", &*m_scene);
		m_colorshader = std::make_unique<shader>("res/shaders/bsc.shader", ShaderType::COLORSHADER);
		m_depthshader = std::make_unique<shader>("res/shaders/Depth.shader", ShaderType::DEPTHSHADER);
		m_shadowdebugshader = std::make_unique<shader>("res/shaders/ShadowDebug.shader", ShaderType::COLORSHADER);
		m_grid = std::make_unique<Grid>();

		m_shadowdebugbuffer = std::make_unique<FrameBuffer>(SHADOW_WIDTH, SHADOW_HEIGHT);
		m_colorbuffer = std::make_unique<FrameBuffer>(800, 800);
		m_depthbuffer = std::make_unique<DepthBuffer>();
	}
	catch (const std::exception& e) {
		LOG(LogLevel::_ERROR, "Failed to initialize model or shader: " + std::string(e.what()));
	}

	//for scene
	m_sceneSize = ImVec2(m_colorbuffer->GetWidth(), m_colorbuffer->GetHeight());

	m_camera = std::make_unique<camera>(800.0f, 800.0f, m_viewPos);
	m_shadowcam = std::make_unique<ShadowCamera>(SHADOW_WIDTH, SHADOW_HEIGHT, m_lightSettings.position, m_camera->GetDirection());
}

testModel::~testModel()
{
	if (m_model) {
		m_model->cleanUp();
	}
}


//main render function 
void testModel::onRender()
{
	m_render->Clear(); //clear main(default freambuffer first)

	renderShadowPass();
	renderColorPass();
	renderShadowDebugPass();

}


void testModel::renderColorPass()
{
	m_colorbuffer->Use();
	m_render->Clear(m_backgroundColor);
	m_render->setViewport(m_sceneSize.x, m_sceneSize.y);

	if (!m_colorshader || !m_colorshader->isValid() || !m_model) {
		return;
	}
	shader::ScopedBind shaderBind(*m_colorshader);


	//m_colorshader->setMat4("u_model", m_modelMatrix);
	m_colorshader->setMat4("u_view", m_camera->GetViewMatrix());
	m_colorshader->setMat4("u_projection", m_camera->GetProjectionMatrix());
	m_colorshader->setMat4("u_lightprojection", m_shadowcam->GetProjectionMatrix());
	m_colorshader->setMat4("u_lightview", m_shadowcam->GetViewMatrix());

	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_modelMatrix)));
	m_colorshader->setMat3("u_normalMatrix", normalMatrix);

	m_colorshader->setVec3("u_viewPos", m_camera->GetCameraPos());

	m_colorshader->setMaterial(m_materialSettings);

	m_colorshader->setLight(
		m_lightSettings.position,
		m_lightSettings.color,
		m_lightSettings.intensity,
		m_lightSettings.constant,
		m_lightSettings.linear,
		m_lightSettings.quadratic
	);

	m_depthbuffer->BindTex(3);

	m_colorshader->setTextures(); // diffuse, normal, specular ,depth

	m_colorshader->setBool("u_useColor", m_renderingSettings.useColor);
	if (m_renderingSettings.useColor) {
		m_colorshader->setVec3("u_color", m_renderingSettings.solidColor);
	}

	m_model->Render(*m_colorshader);
	m_grid->render(*m_camera, m_deltaTime);
	m_colorbuffer->Unuse();
}

void testModel::renderShadowPass() {

	m_depthbuffer->Use();
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	m_depthshader->use();

	m_depthshader->setMat4("u_model", m_modelMatrix);
	m_depthshader->setMat4("u_view", m_shadowcam->GetViewMatrix());
	m_depthshader->setMat4("u_projection", m_shadowcam->GetProjectionMatrix());

	m_model->Render(*m_depthshader);

	m_depthshader->unuse();
	m_depthbuffer->Unsue();
	glCullFace(GL_BACK);
}

void testModel::renderShadowDebugPass()
{
	m_shadowdebugbuffer->Use();
	m_render->Clear();
	m_render->setViewport(SHADOW_WIDTH, SHADOW_HEIGHT);

	m_shadowdebugshader->use();

	m_depthbuffer->BindTex(0);
	m_shadowdebugshader->setInt("u_depthMap", 0);

	m_render->renderQuad();

	m_depthbuffer->UnBindTex();
	m_shadowdebugshader->unuse();
	m_shadowdebugbuffer->Unuse();

}
void testModel::onUpdate(GLFWwindow* window)
{
	m_window = window;

	m_camera->inputs(window, m_speed, 40.0f);
	m_deltaTime = 1.0f / m_performanceStats.fps;
	m_timestep += m_deltaTime;
	if (m_physicsSettings.enableGravity) {


		m_physicsSettings.velocity.y += m_physicsSettings.gravityForce * m_deltaTime;
		m_transformSettings.position += m_physicsSettings.velocity * m_deltaTime;

		if (m_transformSettings.position.y < m_physicsSettings.groundLevel) {
			m_transformSettings.position.y = m_physicsSettings.groundLevel;
			m_physicsSettings.velocity.y = -m_physicsSettings.velocity.y * m_physicsSettings.bounceDamping;
		}
	}

	m_shadowcam->Update(m_lightSettings.position, m_lightSettings.direction);
	updateModelMatrix();

	//temp code for input

	if (glfwGetKey(m_window, GLFW_KEY_P) == GLFW_PRESS && m_timestep > 0.56f) {
		m_timestep = 0.0f;

		switch (m_renderpasstype)
		{
		case RenderPassType::SHADOW_PASS:
			setRenderPass(RenderPassType::COLOR_PASS);
			break;
		case RenderPassType::COLOR_PASS:
			setRenderPass(RenderPassType::SHADOW_PASS);

			break;
		default:
			break;
		}

	}
}

void testModel::updateModelMatrix()
{
	m_modelMatrix = glm::mat4(1.0f);
	m_modelMatrix = glm::translate(m_modelMatrix, m_transformSettings.position);
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.x), glm::vec3(1, 0, 0));
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.y), glm::vec3(0, 1, 0));
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.z), glm::vec3(0, 0, 1));
	m_modelMatrix = glm::scale(m_modelMatrix, m_transformSettings.scale);
}


void testModel::onImguiRender()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create dockspace for better window management

	createDockSpace();
	// Render main control panels
	renderMainControlPanel();
	renderSceneViewport();
	renderAssetBrowser();
	renderPerformancePanel();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void testModel::createDockSpace()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	// Force always open
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id);

	ImGui::End();
}

void testModel::renderMainControlPanel()
{
	ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);

	// Use tabs for better organization
	if (ImGui::BeginTabBar("ControlTabs")) {

		if (ImGui::BeginTabItem("Transform")) {
			renderTransformTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Lighting")) {
			renderLightingTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Material")) {
			renderMaterialTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Physics")) {
			renderPhysicsTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Environment")) {
			renderEnvironmentTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void testModel::renderTransformTab()
{
	// Transform controls with better visual grouping
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.5f, 0.8f, 0.8f));
	if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopStyleColor();

		ImGui::Columns(2, "PositionCols", false);
		ImGui::SetColumnWidth(0, 200);

		ImGui::DragFloat3("##Position", &m_transformSettings.position[0], 0.1f, -100.0f, 100.0f);
		ImGui::NextColumn();

		if (ImGui::Button("Reset##Pos", ImVec2(-1, 0))) {
			m_transformSettings.position = glm::vec3(0.0f);
		}
		ImGui::Columns(1);
	}
	else {
		ImGui::PopStyleColor();
	}

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.5f, 0.3f, 0.8f));
	if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopStyleColor();

		ImGui::Columns(2, "RotationCols", false);
		ImGui::SetColumnWidth(0, 200);

		ImGui::DragFloat3("##Rotation", &m_transformSettings.rotation[0], 0.5f, -180.0f, 180.0f);
		ImGui::NextColumn();

		if (ImGui::Button("Reset##Rot", ImVec2(-1, 0))) {
			m_transformSettings.rotation = glm::vec3(0.0f);
		}
		ImGui::Columns(1);
	}
	else {
		ImGui::PopStyleColor();
	}

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.5f, 0.8f, 0.3f, 0.8f));
	if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopStyleColor();

		ImGui::Columns(2, "ScaleCols", false);
		ImGui::SetColumnWidth(0, 200);

		ImGui::DragFloat3("##Scale", &m_transformSettings.scale[0], 0.1f, 0.01f, 20.0f);
		ImGui::NextColumn();

		if (ImGui::Button("Reset##Sca", ImVec2(-1, 0))) {
			m_transformSettings.scale = glm::vec3(1.0f);
		}
		ImGui::Columns(1);
	}
	else {
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	// Quick actions
	ImGui::Text("Quick Actions:");
	if (ImGui::Button("Reset All Transforms", ImVec2(-1, 0))) {
		m_transformSettings.position = glm::vec3(0.0f);
		m_transformSettings.rotation = glm::vec3(0.0f);
		m_transformSettings.scale = glm::vec3(1.0f);
	}

	// Gizmo controls
	renderGizmoControls();
}

void testModel::renderLightingTab()
{
	// Light position and direction
	if (ImGui::CollapsingHeader("Light Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat3("Position", &m_lightSettings.position[0], 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat3("Direction", &m_lightSettings.direction[0], 0.1f, -1.0f, 1.0f);

		if (ImGui::Button("Align to Camera", ImVec2(-1, 0))) {
			m_lightSettings.position = m_camera->GetCameraPos();
			m_lightSettings.direction = m_camera->GetDirection();
		}
	}

	// Light properties
	if (ImGui::CollapsingHeader("Light Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit3("Color", &m_lightSettings.color[0]);
		ImGui::SliderFloat("Intensity", &m_lightSettings.intensity, 0.0f, 5.0f);

		ImGui::Separator();
		ImGui::Text("Attenuation:");
		ImGui::SliderFloat("Constant", &m_lightSettings.constant, 0.1f, 2.0f);
		ImGui::SliderFloat("Linear", &m_lightSettings.linear, 0.001f, 0.5f, "%.3f");
		ImGui::SliderFloat("Quadratic", &m_lightSettings.quadratic, 0.001f, 0.1f, "%.4f");

		// Attenuation preview
		float distance = glm::length(m_lightSettings.position - m_transformSettings.position);
		float attenuation = 1.0f / (m_lightSettings.constant +
			m_lightSettings.linear * distance +
			m_lightSettings.quadratic * distance * distance);

		ImGui::Text("Distance: %.2f", distance);
		ImGui::ProgressBar(attenuation, ImVec2(-1, 0), ("Attenuation: " + std::to_string(attenuation)).c_str());
	}
}

void testModel::renderMaterialTab()
{
	// Render mode
	if (ImGui::CollapsingHeader("Render Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
		const char* renderModes[] = { "Fill", "Wireframe", "Point" };
		static int currentMode = 0;
		ImGui::Combo("Polygon Mode", &currentMode, renderModes, IM_ARRAYSIZE(renderModes));
		m_renderMode = static_cast<RenderMode>(currentMode);
		m_render->setRenderMode(m_renderMode);

		ImGui::Checkbox("Use Solid Color", &m_renderingSettings.useColor);
		if (m_renderingSettings.useColor) {
			ImGui::ColorEdit3("Color", &m_renderingSettings.solidColor[0]);
		}
	}

	// Material properties
	if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit3("Ambient", &m_materialSettings.ambient[0]);
		ImGui::ColorEdit3("Diffuse", &m_materialSettings.diffuse[0]);
		ImGui::ColorEdit3("Specular", &m_materialSettings.specular[0]);
		ImGui::SliderFloat("Shininess", &m_materialSettings.shininess, 1.0f, 256.0f);
	}

	// Texture mapping
	if (ImGui::CollapsingHeader("Texture Mapping")) {
		ImGui::Checkbox("Normal Mapping", &m_materialSettings.hasNormalMap);
		if (m_materialSettings.hasNormalMap) {
			ImGui::SliderFloat("Normal Strength", &m_materialSettings.normalStrength, 0.0f, 3.0f);
		}
		ImGui::Checkbox("Specular Mapping", &m_materialSettings.hasSpecularMap);
	}
}

void testModel::renderPhysicsTab()
{
	ImGui::Checkbox("Enable Physics", &m_physicsSettings.enableGravity);

	if (m_physicsSettings.enableGravity) {
		ImGui::Separator();

		ImGui::SliderFloat("Gravity", &m_physicsSettings.gravityForce, -20.0f, -0.1f);
		ImGui::SliderFloat("Ground Level", &m_physicsSettings.groundLevel, -10.0f, 10.0f);
		ImGui::SliderFloat("Bounce Damping", &m_physicsSettings.bounceDamping, 0.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
			m_physicsSettings.velocity.x,
			m_physicsSettings.velocity.y,
			m_physicsSettings.velocity.z);

		if (ImGui::Button("Reset Velocity", ImVec2(-1, 0))) {
			m_physicsSettings.velocity = glm::vec3(0.0f);
		}
	}
	else {
		m_physicsSettings.velocity = glm::vec3(0.0f);
	}
}

void testModel::renderEnvironmentTab()
{
	// Camera controls
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		glm::vec3 camPos = m_camera->GetCameraPos();
		glm::vec3 camFront = m_camera->getFront();

		ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
		ImGui::Text("Direction: (%.2f, %.2f, %.2f)", camFront.x, camFront.y, camFront.z);

		ImGui::SliderFloat("Speed", &m_speed, 0.001f, 1.0f, "%.3f");

		if (ImGui::Button("Reset Camera", ImVec2(-1, 0))) {
			m_camera = std::make_unique<camera>(800.0f, 800.0f, glm::vec3(0.0f, 0.0f, 3.0f));
		}
	}

	// Background and grid
	if (ImGui::CollapsingHeader("Background & Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorEdit3("Background", &m_backgroundColor[0]);

		auto& gridSettings = m_grid->getSetting();
		ImGui::Checkbox("Show Grid", &gridSettings.enableGrid);

		if (gridSettings.enableGrid) {
			ImGui::SliderFloat("Grid Intensity", &gridSettings.gridIntensity, 0.0f, 2.0f);
			ImGui::ColorEdit3("Grid Color", &gridSettings.baseColor[0]);
		}
	}

	// Space-time fabric effects
	if (ImGui::CollapsingHeader("Fabric Effects")) {
		auto& settings = m_grid->getSetting();

		ImGui::Checkbox("Enable Animation", &settings.enableAnimation);
		ImGui::Checkbox("Enable Gradient", &settings.enableGradient);

		if (settings.enableAnimation) {
			ImGui::SliderFloat("Wave Amplitude", &settings.waveAmplitude, 0.0f, 2.0f);
			ImGui::SliderFloat("Wave Frequency", &settings.waveFrequency, 0.01f, 0.5f);
		}

		if (settings.enableGradient) {
			ImGui::ColorEdit3("Gradient Color", &settings.gradientColor[0]);
		}

		ImGui::SliderFloat("Fade Distance", &settings.fadeDistance, 10.0f, 200.0f);

		if (ImGui::Button("Reset Fabric", ImVec2(-1, 0))) {
			settings = GridSettings();
		}
	}
}

void testModel::renderSceneViewport()
{
	ImGui::Begin("Scene Viewport", nullptr, ImGuiWindowFlags_None);

	// Viewport controls
	ImGui::BeginGroup();
	ImGui::Text("Render Pass:");
	ImGui::SameLine();

	bool shadowPass = (m_renderpasstype == RenderPassType::SHADOW_PASS);
	bool colorPass = (m_renderpasstype == RenderPassType::COLOR_PASS);

	if (ImGui::RadioButton("Color", colorPass)) {
		setRenderPass(RenderPassType::COLOR_PASS);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Shadow", shadowPass)) {
		setRenderPass(RenderPassType::SHADOW_PASS);
	}
	ImGui::EndGroup();

	ImGui::Separator();

	// Display the appropriate render pass
	ImVec2 availableSize = ImGui::GetContentRegionAvail();
	m_sceneSize = availableSize;

	switch (m_renderpasstype) {
	case RenderPassType::SHADOW_PASS:
		ImGui::Image((ImTextureID)(intptr_t)m_shadowdebugbuffer->GetTextureId(),
			m_sceneSize, ImVec2(0, 1), ImVec2(1, 0));
		break;
	case RenderPassType::COLOR_PASS:
		ImGui::Image((ImTextureID)(intptr_t)m_colorbuffer->GetTextureId(),
			m_sceneSize, ImVec2(0, 1), ImVec2(1, 0));
		m_scene->RenderScenePanel(m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix(), m_currentop);
		break;
	}

	ImGui::End();
}

void testModel::renderAssetBrowser()
{
	ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_None);

	// Model loading
	if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen)) {
		static std::string modelPath = "No model selected";

		ImGui::TextWrapped("Current: %s", modelPath.c_str());

		if (ImGui::Button("Load Model", ImVec2(-1, 0))) {
			std::string selected = FileDial::OpenFile();
			if (!selected.empty()) {
				modelPath = selected;
				loadModel(modelPath);
			}
		}
	}

	// Shader loading
	if (ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_DefaultOpen)) {
		static std::string shaderPath = "No shader selected";

		ImGui::TextWrapped("Current: %s", shaderPath.c_str());

		if (ImGui::Button("Load Shader", ImVec2(-1, 0))) {
			std::string selected = FileDial::OpenFile();
			if (!selected.empty()) {
				shaderPath = selected;
				loadShader(shaderPath);
			}
		}

		if (ImGui::Button("Reload Current", ImVec2(-1, 0))) {
			if (m_colorshader) {
				m_colorshader->reload();
			}
		}

		if (m_colorshader && ImGui::Button("Debug Uniforms", ImVec2(-1, 0))) {
			m_colorshader->printActiveUniforms();
		}
	}

	ImGui::End();
}

void testModel::renderPerformancePanel()
{
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_None);

	m_performanceStats.fps = m_render->updateAndLogFPS(m_window);

	// FPS display with color coding
	ImVec4 fpsColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
	if (m_performanceStats.fps < 30.0f) fpsColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
	else if (m_performanceStats.fps < 60.0f) fpsColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
	ImGui::Text("FPS: %.1f", m_performanceStats.fps);
	ImGui::PopStyleColor();

	ImGui::Text("Frame Time: %.3f ms", 1000.0f / m_performanceStats.fps);

	// FPS graph (simple progress bar for now)
	float fpsNormalized = m_performanceStats.fps / 120.0f; // Normalize to 120 FPS max
	ImGui::ProgressBar(fpsNormalized, ImVec2(-1, 0));

	ImGui::Separator();

	static bool showDemoWindow = false;
	ImGui::Checkbox("Show ImGui Demo", &showDemoWindow);
	if (showDemoWindow) {
		ImGui::ShowDemoWindow(&showDemoWindow);
	}

	ImGui::Separator();
	ImGui::TextDisabled("Enhanced PBR Renderer");
	ImGui::TextDisabled("OpenGL + ImGui");

	// Memory usage (if you have access to system info)
	//ImGui::Separator();
	//ImGui::Text("Render Stats:");
	//ImGui::Text("Triangles: %d", m_model ? m_model->getTriangleCount() : 0);
	//ImGui::Text("Draw Calls: %d", m_render->getDrawCallCount());

	ImGui::End();
}

// Additional utility functions for enhanced UX
void testModel::renderPresetControls()
{
	if (ImGui::CollapsingHeader("Presets")) {
		if (ImGui::Button("Studio Lighting", ImVec2(-1, 0))) {
			m_lightSettings.position = glm::vec3(5.0f, 10.0f, 5.0f);
			m_lightSettings.color = glm::vec3(1.0f, 0.95f, 0.8f);
			m_lightSettings.intensity = 1.2f;
		}

		if (ImGui::Button("Dramatic Lighting", ImVec2(-1, 0))) {
			m_lightSettings.position = glm::vec3(-3.0f, 8.0f, -2.0f);
			m_lightSettings.color = glm::vec3(0.9f, 0.7f, 0.5f);
			m_lightSettings.intensity = 2.0f;
		}

		if (ImGui::Button("Outdoor Scene", ImVec2(-1, 0))) {
			m_lightSettings.position = glm::vec3(10.0f, 20.0f, 10.0f);
			m_lightSettings.color = glm::vec3(1.0f, 1.0f, 0.9f);
			m_lightSettings.intensity = 0.8f;
			m_backgroundColor = glm::vec3(0.53f, 0.81f, 0.92f); // Sky blue
		}
	}
}

//void testModel::renderToolbar()
//{
//    ImGui::Begin("Toolbar", nullptr,
//        ImGuiWindowFlags_NoTitleBar |
//        ImGuiWindowFlags_NoResize |
//        ImGuiWindowFlags_AlwaysAutoResize);
//
//    // Quick action buttons
//    if (ImGui::Button("Reset Scene")) {
//        resetScene();
//    }
//    ImGui::SameLine();
//
//    if (ImGui::Button("Save View")) {
//        saveCurrentView();
//    }
//    ImGui::SameLine();
//
//    if (ImGui::Button("Load View")) {
//        loadSavedView();
//    }
//
//    ImGui::End();
//}

void testModel::renderGizmoControls()
{
	if (ImGui::CollapsingHeader("Gizmo Controls")) {

		ImGui::Text("Gizmo Mode:");
		if (ImGui::RadioButton("Translate##Gizmos", m_currentop == ImGuizmo::TRANSLATE))
			m_currentop = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate##Gizmos", m_currentop == ImGuizmo::ROTATE))
			m_currentop = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale##Gizmos", m_currentop == ImGuizmo::SCALE))
			m_currentop = ImGuizmo::SCALE;

		// Apply gizmo transformations
	}
}

void testModel::renderGizmoManipulator(ImGuizmo::OPERATION operation)
{
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();

	float windowheight = (float)ImGui::GetWindowHeight();
	float windowwidth = (float)ImGui::GetWindowWidth();

	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowwidth, windowheight);

	glm::mat4 gizmoMatrix = m_modelMatrix;

	ImGuizmo::Manipulate(
		glm::value_ptr(m_camera->GetViewMatrix()),
		glm::value_ptr(m_camera->GetProjectionMatrix()),
		operation,
		ImGuizmo::LOCAL,
		glm::value_ptr(gizmoMatrix)
	);

	if (ImGuizmo::IsUsing()) {
		// Decompose the gizmo matrix and update transform settings
		glm::vec3 skew, scale, position;
		glm::vec4 perspective;
		glm::quat rotation;

		glm::decompose(gizmoMatrix, scale, rotation, position, skew, perspective);

		m_transformSettings.position = position;
		m_transformSettings.scale = scale;
		m_transformSettings.rotation = glm::degrees(glm::eulerAngles(rotation));
	}
}

// Helper function to create styled separator with text
void testModel::styledSeparator(const char* text)
{
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImVec2 textSize = ImGui::CalcTextSize(text);
	ImVec2 windowSize = ImGui::GetWindowSize();
	ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 1.0f, 1.0f));
	ImGui::Text("%s", text);
	ImGui::PopStyleColor();

	ImGui::Spacing();
}

// Enhanced styling in constructor
void testModel::setupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// Rounded corners
	style.WindowRounding = 6.0f;
	style.ChildRounding = 3.0f;
	style.FrameRounding = 3.0f;
	style.PopupRounding = 3.0f;
	style.ScrollbarRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 3.0f;

	// Padding and spacing
	style.WindowPadding = ImVec2(12, 8);
	style.FramePadding = ImVec2(8, 4);
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.IndentSpacing = 20.0f;

	// Borders
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;

	// Modern dark theme
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.0f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.0f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.0f);
	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
}




// helpers  

void testModel::loadModel(const std::string& filepath)
{
	try {
		if (m_model) {
			m_model->cleanUp();
		}

		m_model = std::make_unique<Model>(filepath);

		// Reset transform when loading new model
		m_transformSettings.position = glm::vec3(0.0f);
		m_transformSettings.rotation = glm::vec3(0.0f);
		m_transformSettings.scale = glm::vec3(1.0f);

		LOG(LogLevel::_INFO, "Successfully loaded model: " + filepath);
	}
	catch (const std::exception& e) {
		LOG(LogLevel::_ERROR, "Failed to load model '" + filepath + "': " + std::string(e.what()));
	}
}

void testModel::loadShader(const std::string& filepath)
{
	try {
		auto newShader = std::make_unique<shader>(filepath);

		if (newShader->isValid()) {
			m_colorshader = std::move(newShader);
			LOG(LogLevel::_INFO, "Successfully loaded shader: " + filepath);
		}
		else {
			LOG(LogLevel::_ERROR, "Shader validation failed: " + filepath);
		}
	}
	catch (const std::exception& e) {
		LOG(LogLevel::_ERROR, "Failed to load shader '" + filepath + "': " + std::string(e.what()));
	}
}

