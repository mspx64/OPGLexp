
#include"Logger.h"
#include"tests/testmodel.h"

//TODO
//Fix the diffuse texture mapping issue 
// shadow implementation 

#ifdef LGT_DEBUG
#define MAIN void main()
#define MAIN_RETURN return 
#else
#define MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#define MAIN_RETURN return 1
#endif 

MAIN
{
   GLFWwindow * window;
   Logger::GetInstance().Init();
   /* Initialize the library */
   if (!glfwInit())
	   MAIN_RETURN;
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   float height = 1080, width = 1920;

   window = glfwCreateWindow(width,height,"Lightnig", NULL, NULL);
   /* Create a windowed mode window and its OpenGL context */
   if (!window)
	   MAIN_RETURN;

   /* Make the window's context current */
   glfwMakeContextCurrent(window);
   glfwSwapInterval(0);
   if (glewInit() != GLEW_OK)
	  Logger::GetInstance().Log(LogLevel::_ERROR, "Failed to initialize GLEW");


   //initilize imgui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();

   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("# version 450");

   //gl settings
   glEnable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_DEPTH_TEST);

   Logger::GetInstance().SetLogFile("log.txt");
   LOG(LogLevel::DEBUG, "every one is also fuckef up in there own way");

   Test* test = new testModel;

   //game loop
   while (!glfwWindowShouldClose(window))
   {
	   /* Render here */
	   test->onUpdate(window);
	   test->onRender();
	   test->onImguiRender();

	   /* Swap front and back buffers */
	   glfwSwapBuffers(window);

	   /* Poll for and process events */
	   glfwPollEvents();
   }
   delete test;
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   glfwDestroyWindow(window);
   glfwTerminate();
}