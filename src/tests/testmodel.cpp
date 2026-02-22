#include "testmodel.h"
#include "helpers/Filedial.h"
#include "Renderer/camera.h"


// Constructor / Destructor
testModel::testModel() : m_speed(0.030f)
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	setupImGuiStyle();

	m_modelMatrix = glm::mat4(1.0f);

	m_lightSettings = {
		glm::vec3(0.0f, -1.0f, 0.0f),  // direction
		glm::vec3(2.0f,  4.0f, 2.0f),  // position
		glm::vec3(1.0f,  1.0f, 1.0f),  // color
		1.0f,                           // intensity
		1.0f,                           // constant
		0.09f,                          // linear
		0.032f                          // quadratic
	};

	m_render = std::make_unique<renderer>();
	m_render->logGlVersion();
	m_render->Initauad();

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
		LOG(LogLevel::_ERROR, "Initialization failed: " + std::string(e.what()));
	}

	m_sceneSize = ImVec2((float)m_colorbuffer->GetWidth(), (float)m_colorbuffer->GetHeight());
	m_camera = std::make_unique<camera>(800.0f, 800.0f, m_viewPos);
	m_shadowcam = std::make_unique<ShadowCamera>(SHADOW_WIDTH, SHADOW_HEIGHT,
		m_lightSettings.position, m_camera->GetDirection());
}

testModel::~testModel()
{
	if (m_model)
		m_model->cleanUp();
}


// Main loop hooks


void testModel::onRender()
{
	renderShadowPass();
	renderShadowDebugPass();
	renderColorPass();
}

void testModel::onUpdate(GLFWwindow* window)
{
	m_window = window;

	m_camera->inputs(window, m_speed, 40.0f);

	// Guard against division by zero on first frame
	m_deltaTime = (m_performanceStats.fps > 0.0f) ? 1.0f / m_performanceStats.fps : 0.016f;
	m_timestep += m_deltaTime;

	if (m_physicsSettings.enableGravity)
	{
		m_physicsSettings.velocity.y += m_physicsSettings.gravityForce * m_deltaTime;
		m_transformSettings.position += m_physicsSettings.velocity * m_deltaTime;

		if (m_transformSettings.position.y < m_physicsSettings.groundLevel)
		{
			m_transformSettings.position.y = m_physicsSettings.groundLevel;
			m_physicsSettings.velocity.y = -m_physicsSettings.velocity.y * m_physicsSettings.bounceDamping;
		}
	}

	m_shadowcam->Update(m_lightSettings.position, m_lightSettings.direction);
	updateModelMatrix();

	// Toggle render pass with P key (debounced)
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && m_timestep > 0.56f)
	{
		m_timestep = 0.0f;
		setRenderPass(m_renderpasstype == RenderPassType::COLOR_PASS
			? RenderPassType::SHADOW_PASS
			: RenderPassType::COLOR_PASS);
	}

	int fbW = m_colorbuffer->GetWidth();
	int fbH = m_colorbuffer->GetHeight();

	if ((int)m_sceneSize.x != fbW || (int)m_sceneSize.y != fbH)
	{
		m_colorbuffer->Resize((int)m_sceneSize.x, (int)m_sceneSize.y);
		m_shadowdebugbuffer->Resize((int)m_sceneSize.x, (int)m_sceneSize.y);
		m_camera->setAspect((int)m_sceneSize.x, (int)m_sceneSize.y);
	}

}

void testModel::onImguiRender()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame(); // ← must be called every frame after NewFrame

	createDockSpace();
	renderMainControlPanel();
	renderSceneViewport();   // gizmo is drawn inside here
	renderAssetBrowser();
	renderPerformancePanel();
	renderGizmoControls();
	m_scene->RenderScenePanel(); // entity list panel

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// Render passes


void testModel::renderColorPass()
{
	if (!m_colorshader || !m_colorshader->isValid())
		return;

	// Resize framebuffer + update camera aspect if the viewport panel changed size


	m_colorbuffer->Use();
	m_render->Clear(m_backgroundColor);
	m_render->setViewport((int)m_sceneSize.x, (int)m_sceneSize.y);

	shader::ScopedBind shaderBind(*m_colorshader);

	// Matrices
	m_colorshader->setMat4("u_view", m_camera->GetViewMatrix());
	m_colorshader->setMat4("u_projection", m_camera->GetProjectionMatrix());
	m_colorshader->setMat4("u_lightprojection", m_shadowcam->GetProjectionMatrix());
	m_colorshader->setMat4("u_lightview", m_shadowcam->GetViewMatrix());

	// Normal matrix — derived from model matrix for the standalone model
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
	m_colorshader->setTextures();

	m_colorshader->setBool("u_useColor", m_renderingSettings.useColor);
	if (m_renderingSettings.useColor)
		m_colorshader->setVec3("u_color", m_renderingSettings.solidColor);

	// Scene renders each entity with its own u_model (set inside Scene::Render)
	m_scene->Render(*m_colorshader);
	m_grid->render(*m_camera, m_deltaTime);

	m_colorbuffer->Unuse();
}

void testModel::renderShadowPass()
{
	m_depthbuffer->Use();
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	m_depthshader->use();
	// u_model is set per-entity inside Scene::Render
	m_depthshader->setMat4("u_view", m_shadowcam->GetViewMatrix());
	m_depthshader->setMat4("u_projection", m_shadowcam->GetProjectionMatrix());

	m_scene->Render(*m_depthshader);

	m_depthshader->unuse();
	m_depthbuffer->Unsue();
	glCullFace(GL_BACK);
}

void testModel::renderShadowDebugPass()
{
	m_shadowdebugbuffer->Use();
	m_render->Clear();
	m_render->setViewport(m_sceneSize.x, m_sceneSize.y);
	m_shadowdebugshader->use();
	m_depthbuffer->BindTex(0);
	m_shadowdebugshader->setInt("u_depthMap", 0);
	m_render->renderQuad();
	m_depthbuffer->UnBindTex();
	m_shadowdebugshader->unuse();
	m_shadowdebugbuffer->Unuse();
}


// Transform


void testModel::updateModelMatrix()
{
	m_modelMatrix = glm::mat4(1.0f);
	m_modelMatrix = glm::translate(m_modelMatrix, m_transformSettings.position);
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.x), { 1,0,0 });
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.y), { 0,1,0 });
	m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_transformSettings.rotation.z), { 0,0,1 });
	m_modelMatrix = glm::scale(m_modelMatrix, m_transformSettings.scale);
}


// ImGui — layout


void testModel::createDockSpace()
{
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	const ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos);
	ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("DockSpace", nullptr, flags);
	ImGui::PopStyleVar(3);

	ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
	ImGui::End();
}

void testModel::renderMainControlPanel()
{
	ImGui::Begin("Scene Controls");

	if (ImGui::BeginTabBar("ControlTabs"))
	{
		if (ImGui::BeginTabItem("Lighting")) { renderLightingTab();      ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Material")) { renderMaterialTab();      ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Physics")) { renderPhysicsTab();       ImGui::EndTabItem(); }
		if (ImGui::BeginTabItem("Environment")) { renderEnvironmentTab();   ImGui::EndTabItem(); }
		ImGui::EndTabBar();
	}
	ImGui::End();
}


// ImGui — scene viewport (owns the gizmo)
void testModel::renderSceneViewport()
{
	ImGui::Begin("Scene Viewport");

	// Pass selector
	ImGui::Text("Render Pass:");
	ImGui::SameLine();
	if (ImGui::RadioButton("Color", m_renderpasstype == RenderPassType::COLOR_PASS))
		setRenderPass(RenderPassType::COLOR_PASS);
	ImGui::SameLine();
	if (ImGui::RadioButton("Shadow", m_renderpasstype == RenderPassType::SHADOW_PASS))
		setRenderPass(RenderPassType::SHADOW_PASS);
	ImGui::Separator();

	// Capture viewport metrics BEFORE rendering the image
	ImVec2 available = ImGui::GetContentRegionAvail();
	if (available.x > 1.0f && available.y > 1.0f)
		m_sceneSize = available;

	// Content-region origin in screen space (correct origin for ImGuizmo::SetRect)
	ImVec2 contentMin = ImGui::GetWindowPos();
	contentMin.x += ImGui::GetWindowContentRegionMin().x;
	contentMin.y += ImGui::GetWindowContentRegionMin().y;

	switch (m_renderpasstype)
	{
	case RenderPassType::SHADOW_PASS:
		ImGui::Image((ImTextureID)(intptr_t)m_shadowdebugbuffer->GetTextureId(),
			m_sceneSize, ImVec2(0, 1), ImVec2(1, 0));
		break;

	case RenderPassType::COLOR_PASS:
		ImGui::Image((ImTextureID)(intptr_t)m_colorbuffer->GetTextureId(),
			m_sceneSize, ImVec2(0, 1), ImVec2(1, 0));

		// Gizmo must be drawn AFTER the image, INSIDE this same Begin/End block
		m_scene->RenderGizmo(
			m_camera->GetViewMatrix(),
			m_camera->GetProjectionMatrix(),
			m_currentop,
			contentMin,   // correct screen-space origin
			m_sceneSize
		);
		break;
	}

	ImGui::End();
}


void testModel::renderLightingTab()
{
	if (ImGui::CollapsingHeader("Light Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Position", &m_lightSettings.position[0], 0.1f, -100, 100);
		ImGui::DragFloat3("Direction", &m_lightSettings.direction[0], 0.01f, -1, 1);

		if (ImGui::Button("Align to Camera", ImVec2(-1, 0)))
		{
			m_lightSettings.position = m_camera->GetCameraPos();
			m_lightSettings.direction = m_camera->GetDirection();
		}
	}

	if (ImGui::CollapsingHeader("Light Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ColorEdit3("Color", &m_lightSettings.color[0]);
		ImGui::SliderFloat("Intensity", &m_lightSettings.intensity, 0, 5);
		ImGui::Separator();
		ImGui::Text("Attenuation:");
		ImGui::SliderFloat("Constant", &m_lightSettings.constant, 0.1f, 2.0f);
		ImGui::SliderFloat("Linear", &m_lightSettings.linear, 0.001f, 0.5f, "%.3f");
		ImGui::SliderFloat("Quadratic", &m_lightSettings.quadratic, 0.001f, 0.1f, "%.4f");

		float d = glm::length(m_lightSettings.position - m_transformSettings.position);
		float atten = 1.0f / (m_lightSettings.constant +
			m_lightSettings.linear * d +
			m_lightSettings.quadratic * d * d);
		ImGui::Text("Distance: %.2f", d);
		ImGui::ProgressBar(atten, ImVec2(-1, 0),
			("Attenuation: " + std::to_string(atten)).c_str());
	}
}

void testModel::renderMaterialTab()
{
	if (ImGui::CollapsingHeader("Render Mode", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* modes[] = { "Fill","Wireframe","Point" };
		static int currentMode = 0;
		ImGui::Combo("Polygon Mode", &currentMode, modes, IM_ARRAYSIZE(modes));
		m_renderMode = static_cast<RenderMode>(currentMode);
		m_render->setRenderMode(m_renderMode);

		ImGui::Checkbox("Use Solid Color", &m_renderingSettings.useColor);
		if (m_renderingSettings.useColor)
			ImGui::ColorEdit3("Color##solid", &m_renderingSettings.solidColor[0]);
	}

	if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ColorEdit3("Ambient", &m_materialSettings.ambient[0]);
		ImGui::ColorEdit3("Diffuse", &m_materialSettings.diffuse[0]);
		ImGui::ColorEdit3("Specular", &m_materialSettings.specular[0]);
		ImGui::SliderFloat("Shininess", &m_materialSettings.shininess, 1, 256);
	}

	if (ImGui::CollapsingHeader("Texture Mapping"))
	{
		ImGui::Checkbox("Normal Mapping", &m_materialSettings.hasNormalMap);
		if (m_materialSettings.hasNormalMap)
			ImGui::SliderFloat("Normal Strength", &m_materialSettings.normalStrength, 0, 3);
		ImGui::Checkbox("Specular Mapping", &m_materialSettings.hasSpecularMap);
	}
}

void testModel::renderPhysicsTab()
{
	ImGui::Checkbox("Enable Physics", &m_physicsSettings.enableGravity);

	if (m_physicsSettings.enableGravity)
	{
		ImGui::Separator();
		ImGui::SliderFloat("Gravity", &m_physicsSettings.gravityForce, -20, -0.1f);
		ImGui::SliderFloat("Ground Level", &m_physicsSettings.groundLevel, -10, 10);
		ImGui::SliderFloat("Bounce Damping", &m_physicsSettings.bounceDamping, 0, 1);
		ImGui::Separator();
		ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
			m_physicsSettings.velocity.x,
			m_physicsSettings.velocity.y,
			m_physicsSettings.velocity.z);
		if (ImGui::Button("Reset Velocity", ImVec2(-1, 0)))
			m_physicsSettings.velocity = glm::vec3(0);
	}
	else
	{
		m_physicsSettings.velocity = glm::vec3(0);
	}
}

void testModel::renderEnvironmentTab()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		glm::vec3 pos = m_camera->GetCameraPos();
		glm::vec3 front = m_camera->getFront();
		ImGui::Text("Position:  (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		ImGui::Text("Direction: (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
		ImGui::SliderFloat("Speed", &m_speed, 0.001f, 1.0f, "%.3f");
		if (ImGui::Button("Reset Camera", ImVec2(-1, 0)))
			m_camera = std::make_unique<camera>(800.0f, 800.0f, glm::vec3(0, 0, 3));
	}

	if (ImGui::CollapsingHeader("Background & Grid", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::ColorEdit3("Background", &m_backgroundColor[0]);

		auto& gs = m_grid->getSetting();
		ImGui::Checkbox("Show Grid", &gs.enableGrid);
		if (gs.enableGrid)
		{
			ImGui::SliderFloat("Grid Intensity", &gs.gridIntensity, 0, 2);
			ImGui::ColorEdit3("Grid Color", &gs.baseColor[0]);
		}
	}

	if (ImGui::CollapsingHeader("Fabric Effects"))
	{
		auto& gs = m_grid->getSetting();
		ImGui::Checkbox("Enable Animation", &gs.enableAnimation);
		ImGui::Checkbox("Enable Gradient", &gs.enableGradient);
		if (gs.enableAnimation)
		{
			ImGui::SliderFloat("Wave Amplitude", &gs.waveAmplitude, 0, 2);
			ImGui::SliderFloat("Wave Frequency", &gs.waveFrequency, 0.01f, 0.5f);
		}
		if (gs.enableGradient)
			ImGui::ColorEdit3("Gradient Color", &gs.gradientColor[0]);
		ImGui::SliderFloat("Fade Distance", &gs.fadeDistance, 10, 200);
		if (ImGui::Button("Reset Fabric", ImVec2(-1, 0)))
			gs = GridSettings();
	}
}

void testModel::renderGizmoControls()
{
	ImGui::Begin("Gizmo Controls");
	if (ImGui::CollapsingHeader("Gizmo##Controls"))
	{
		ImGui::Text("Mode:");
		if (ImGui::RadioButton("Translate##G", m_currentop == ImGuizmo::TRANSLATE)) m_currentop = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate##G", m_currentop == ImGuizmo::ROTATE))    m_currentop = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale##G", m_currentop == ImGuizmo::SCALE))     m_currentop = ImGuizmo::SCALE;
	}
	ImGui::End();
}


// ImGui — other panels


void testModel::renderAssetBrowser()
{
	ImGui::Begin("Asset Browser");

	if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static std::string modelPath = "No model selected";
		ImGui::TextWrapped("Current: %s", modelPath.c_str());
		if (ImGui::Button("Load Model", ImVec2(-1, 0)))
		{
			std::string sel = FileDial::OpenFile();
			if (!sel.empty()) { modelPath = sel; loadModel(modelPath); }
		}
	}

	if (ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static std::string shaderPath = "No shader selected";
		ImGui::TextWrapped("Current: %s", shaderPath.c_str());
		if (ImGui::Button("Load Shader", ImVec2(-1, 0)))
		{
			std::string sel = FileDial::OpenFile();
			if (!sel.empty()) { shaderPath = sel; loadShader(shaderPath); }
		}
		if (ImGui::Button("Reload Current", ImVec2(-1, 0)) && m_colorshader)
			m_colorshader->reload();
		if (m_colorshader && ImGui::Button("Debug Uniforms", ImVec2(-1, 0)))
			m_colorshader->printActiveUniforms();
	}

	ImGui::End();
}

void testModel::renderPerformancePanel()
{
	ImGui::Begin("Performance");

	m_performanceStats.fps = m_render->updateAndLogFPS(m_window);
	float fps = m_performanceStats.fps;

	ImVec4 col = (fps < 30) ? ImVec4(1, 0, 0, 1) : (fps < 60) ? ImVec4(1, 1, 0, 1) : ImVec4(0, 1, 0, 1);
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	ImGui::Text("FPS: %.1f", fps);
	ImGui::PopStyleColor();

	ImGui::Text("Frame Time: %.3f ms", fps > 0 ? 1000.0f / fps : 0.0f);
	ImGui::ProgressBar(fps / 120.0f, ImVec2(-1, 0));
	ImGui::Separator();

	static bool showDemo = false;
	ImGui::Checkbox("Show ImGui Demo", &showDemo);
	if (showDemo) ImGui::ShowDemoWindow(&showDemo);

	ImGui::Separator();
	ImGui::TextDisabled("OpenGL + ImGui Renderer");

	ImGui::End();
}

void testModel::renderPresetControls()
{
	if (ImGui::CollapsingHeader("Presets"))
	{
		if (ImGui::Button("Studio Lighting", ImVec2(-1, 0)))
		{
			m_lightSettings.position = { 5,10, 5 };
			m_lightSettings.color = { 1,0.95f,0.8f };
			m_lightSettings.intensity = 1.2f;
		}
		if (ImGui::Button("Dramatic Lighting", ImVec2(-1, 0)))
		{
			m_lightSettings.position = { -3, 8,-2 };
			m_lightSettings.color = { 0.9f,0.7f,0.5f };
			m_lightSettings.intensity = 2.0f;
		}
		if (ImGui::Button("Outdoor Scene", ImVec2(-1, 0)))
		{
			m_lightSettings.position = { 10,20,10 };
			m_lightSettings.color = { 1,1,0.9f };
			m_lightSettings.intensity = 0.8f;
			m_backgroundColor = { 0.53f,0.81f,0.92f };
		}
	}
}

void testModel::renderToolbar()
{
	ImGui::Begin("Toolbar", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::Button("Reset Scene")) { /* resetScene(); */ }
	ImGui::SameLine();
	if (ImGui::Button("Save View")) { /* saveCurrentView(); */ }
	ImGui::SameLine();
	if (ImGui::Button("Load View")) { /* loadSavedView(); */ }
	ImGui::End();
}

void testModel::styledSeparator(const char* text)
{
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImVec2 ts = ImGui::CalcTextSize(text);
	ImVec2 ws = ImGui::GetWindowSize();
	ImGui::SetCursorPosX((ws.x - ts.x) * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 1.0f, 1.0f));
	ImGui::Text("%s", text);
	ImGui::PopStyleColor();
	ImGui::Spacing();
}


// ImGui style


void testModel::setupImGuiStyle()
{
	ImGuiStyle& s = ImGui::GetStyle();
	s.WindowRounding = 6.0f;
	s.ChildRounding = 3.0f;
	s.FrameRounding = 3.0f;
	s.PopupRounding = 3.0f;
	s.ScrollbarRounding = 3.0f;
	s.GrabRounding = 3.0f;
	s.TabRounding = 3.0f;
	s.WindowPadding = ImVec2(12, 8);
	s.FramePadding = ImVec2(8, 4);
	s.ItemSpacing = ImVec2(8, 4);
	s.ItemInnerSpacing = ImVec2(4, 4);
	s.IndentSpacing = 20.0f;
	s.WindowBorderSize = 1.0f;
	s.ChildBorderSize = 1.0f;
	s.FrameBorderSize = 0.0f;

	ImVec4* c = s.Colors;
	c[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.94f);
	c[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
	c[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	c[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	c[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
	c[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	c[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	c[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
	c[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	c[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
	c[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	c[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
	c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
	c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);
	c[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	c[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.0f);
	c[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	c[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	c[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	c[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.0f);
	c[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	c[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	c[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
	c[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
	c[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	c[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.0f);
	c[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	c[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
}


// Asset loading helpers


void testModel::loadModel(const std::string& filepath)
{
	try {
		if (m_model) m_model->cleanUp();
		m_model = std::make_unique<Model>(filepath);
		m_transformSettings = TransformSettings{}; // reset to defaults
		LOG(LogLevel::_INFO, "Loaded model: " + filepath);
	}
	catch (const std::exception& e) {
		LOG(LogLevel::_ERROR, "Failed to load model '" + filepath + "': " + e.what());
	}
}

void testModel::loadShader(const std::string& filepath)
{
	try {
		auto s = std::make_unique<shader>(filepath);
		if (s->isValid())
		{
			m_colorshader = std::move(s);
			LOG(LogLevel::_INFO, "Loaded shader: " + filepath);
		}
		else
		{
			LOG(LogLevel::_ERROR, "Shader invalid: " + filepath);
		}
	}
	catch (const std::exception& e) {
		LOG(LogLevel::_ERROR, "Failed to load shader '" + filepath + "': " + e.what());
	}
}