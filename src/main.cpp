
#include "Renderer/ErrorReporting.h"
#include "Renderer/Scene.h"
#include "Renderer/Camera.h"
#include "Renderer/Renderer.h"
#include "UI/Editor.h"
#include "helpers/Logger.h"

#define MAIN        void main()
#define MAIN_RETURN return

int main() {
    GLFWwindow* window;

    CORE_LOG_INIT();

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int height = 1080, width = 1920;

    window = glfwCreateWindow(width, height, "Lightnig", NULL, NULL);
    assert(window);
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        CORE_ERROR("Failed to initialize GLAD");
    }
    enableReportGlErrors();
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Editor::ApplyProfessionalTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("# version 460");

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    CORE_INFO("{}", (char*)glGetString(GL_VENDOR));
    CORE_INFO("{}", (char*)glGetString(GL_RENDERER));

    lgt::Scene  scene;
    lgt::Camera camera((int)width, (int)height, glm::vec3(0.0f));

    // Models are now loaded dynamically via the Asset Browser panel at runtime

    int           currentMode = 0;
    lgt::Renderer renderer(&scene, &camera);
    renderer.init();
    
    lgt::Grid grid;
    lgt::FrameBuffer framebuffer(width, height);
    float camSpeed = 0.001f;
    float camSensitivity = 20.0f;
    float deltaTime = 0.016f; // mock deltaTime for now

    bool rKeyWasPressed = false;
    bool fKeyWasPressed = false;

    // game loopx
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Default background color for the dockspace
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update(window, camSpeed, camSensitivity);
        // scene.Update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Viewport Panel & 3D Rendering
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if (viewportSize.x > 0.0f && viewportSize.y > 0.0f) {
            // Resize FBO and Camera if the viewport changed
            if (framebuffer.GetWidth() != static_cast<int>(viewportSize.x) || framebuffer.GetHeight() != static_cast<int>(viewportSize.y)) {
                framebuffer.Resize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
                camera.setAspect(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
            }

            // 3D Scene Rendering to FBO
            framebuffer.Use();
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Viewport background color
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            renderer.setViewport(framebuffer.GetWidth(), framebuffer.GetHeight());
            renderer.render();
            grid.render(camera, deltaTime);
            
            framebuffer.Unuse();

            // Display FBO texture in ImGui
            ImGui::Image((ImTextureID)(intptr_t)framebuffer.GetTextureId(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
        ImGui::PopStyleVar();

        Editor::DrawMaterialEditorPanel();
        Editor::DrawSceneHierarchyPanel(&scene);
        Editor::DrawEnvironmentPanel(&grid, &renderer);
        Editor::DrawCameraPanel(&camera, &camSpeed, &camSensitivity);
        Editor::DrawAssetBrowserPanel(&scene);
        Editor::DrawInspectorPanel();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyWasPressed) {
            renderer.shutdown();
            renderer.init();
            rKeyWasPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && rKeyWasPressed)
            rKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyWasPressed) {
            currentMode = (currentMode + 1) % 11;
            renderer.setDebugMode((lgt::DebugMode)(currentMode));
            fKeyWasPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && fKeyWasPressed)
            fKeyWasPressed = false;

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}