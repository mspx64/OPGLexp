
#include "Renderer/ErrorReporting.h"
#include "Renderer/Scene.h"
#include "Renderer/camera.h"
#include "Renderer/renderer.h"
#include "UI/Editor.h"
#include "helpers/Logger.h"

#define MAIN        void main()
#define MAIN_RETURN return

void main() {
    GLFWwindow* window;

    CORE_LOG_INIT();

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    float height = 1080, width = 1920;

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

    // scene.LoadGltf("res/modles/Lanten/lantern_fbx.fbx");
    // scene.LoadGltf("res/modles/Helmet/DamagedHelmet.gltf");
    scene.LoadGltf("res/modles/sopnza_palace/sponza_palace.gltf");
    //  scene.LoadGltf("res/modles/car/scene.gltf");

    int           currentMode = 0;
    lgt::Renderer renderer(&scene, &camera);
    renderer.init();

    bool rKeyWasPressed = false;
    bool fKeyWasPressed = false;

    // game loopx
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        camera.update(window, 0.001, 20);
        // scene.Update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Editor::DrawMaterialEditorPanel();

        ImGui::Render();
        renderer.render();

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
}