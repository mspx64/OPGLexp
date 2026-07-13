
#include "Renderer/ErrorReporting.h"
#include "Renderer/Scene.h"
#include "Renderer/Camera.h"
#include "Renderer/Renderer.h"
#include "UI/Editor.h"
#include "Helpers/Logger.h"
#include <fstream>

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
    
    // Load a professional-looking font at a larger size
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    // Segoe UI is practically the Windows default font and looks very clean. 
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, &fontConfig);
    
    Editor::ApplyProfessionalTheme();

    // Check if imgui.ini exists, if not, load default layout
    if (io.IniFilename) {
        std::ifstream iniFile(io.IniFilename);
        if (!iniFile.good()) {
            const char* default_layout = R"(
[Window][WindowOverViewport_11111111]
Pos=0,0
Size=1920,1055
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Viewport]
Pos=363,0
Size=1210,718
Collapsed=0
DockId=0x00000009,0

[Window][Material Editor]
Pos=1575,0
Size=345,562
Collapsed=0
DockId=0x00000003,0

[Window][Scene Hierarchy]
Pos=0,0
Size=361,549
Collapsed=0
DockId=0x00000007,0

[Window][Environment & Renderer]
Pos=1575,564
Size=345,491
Collapsed=0
DockId=0x00000004,0

[Window][Camera Controls]
Pos=1575,564
Size=345,491
Collapsed=0
DockId=0x00000004,1

[Window][Asset Browser]
Pos=363,720
Size=503,335
Collapsed=0
DockId=0x0000000B,0

[Window][Inspector]
Pos=0,551
Size=361,504
Collapsed=0
DockId=0x00000008,0

[Window][Console]
Pos=868,720
Size=705,335
Collapsed=0
DockId=0x0000000C,0

[Docking][Data]
DockSpace         ID=0x08BD597D Window=0x1BBC0F80 Pos=0,0 Size=1920,1055 Split=X
  DockNode        ID=0x00000005 Parent=0x08BD597D SizeRef=361,1055 Split=Y Selected=0xB8729153
    DockNode      ID=0x00000007 Parent=0x00000005 SizeRef=473,549 Selected=0xB8729153
    DockNode      ID=0x00000008 Parent=0x00000005 SizeRef=473,504 Selected=0x36DC96AB
  DockNode        ID=0x00000006 Parent=0x08BD597D SizeRef=1557,1055 Split=X
    DockNode      ID=0x00000001 Parent=0x00000006 SizeRef=1210,1055 Split=Y Selected=0xC450F867
      DockNode    ID=0x00000009 Parent=0x00000001 SizeRef=1107,718 CentralNode=1 Selected=0xC450F867
      DockNode    ID=0x0000000A Parent=0x00000001 SizeRef=1107,335 Split=X Selected=0x36AF052B
        DockNode  ID=0x0000000B Parent=0x0000000A SizeRef=503,335 Selected=0x36AF052B
        DockNode  ID=0x0000000C Parent=0x0000000A SizeRef=705,335 Selected=0xEA83D666
    DockNode      ID=0x00000002 Parent=0x00000006 SizeRef=345,1055 Split=Y Selected=0x3D0FF072
      DockNode    ID=0x00000003 Parent=0x00000002 SizeRef=336,562 Selected=0x3D0FF072
      DockNode    ID=0x00000004 Parent=0x00000002 SizeRef=336,491 Selected=0xBF434FD5
)";
            ImGui::LoadIniSettingsFromMemory(default_layout);
        }
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("# version 460");

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    CORE_INFO("{}", (char*)glGetString(GL_VENDOR));
    CORE_INFO("{}", (char*)glGetString(GL_RENDERER));

    lgt::Scene  scene;
    lgt::Camera camera((int)width, (int)height, glm::vec3(0.0f, 1.5f, 3.0f));

    // Models are now loaded dynamically via the Asset Browser panel at runtime


    lgt::Renderer renderer(&scene, &camera);
    renderer.init();
    
    lgt::Grid grid;
    lgt::FrameBuffer framebuffer(width, height);
    float camSpeed = 0.05f;
    float camSensitivity = 20.0f;
    float deltaTime = 0.016f; // mock deltaTime for now

    bool rKeyWasPressed = false;
    bool fKeyWasPressed = false;

    // game loopx
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Default background color for the dockspace
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // scene.Update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Viewport Panel & 3D Rendering
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        bool viewportHovered = ImGui::IsWindowHovered();
        camera.update(window, camSpeed, camSensitivity, viewportHovered);
        
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
        Editor::DrawConsolePanel();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyWasPressed) {
            renderer.shutdown();
            renderer.init();
            rKeyWasPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && rKeyWasPressed)
            rKeyWasPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyWasPressed) {
            int currentMode = (int)renderer.getDebugMode();
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
