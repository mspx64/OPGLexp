# 🌌 OpenGL Experimental Renderer

This project is an **experimental renderer** built with **C++ and OpenGL**, created as a personal learning journey into the world of modern graphics programming and real-time rendering pipelines.

The long-term goal is to evolve this project into a **Physically Based Renderer (PBR)** from scratch. For now, it serves as a sandbox for experimenting with rendering techniques, shaders, and scene management systems.

---
## screenshots
<img width="1918" height="1075" alt="Screenshot 2025-10-25 230854" src="https://github.com/user-attachments/assets/fef2ce93-eb4c-4744-adce-cf77586e2f7f" />
<img width="1750" height="1034" alt="image" src="https://github.com/user-attachments/assets/94e25bba-bb9b-4363-aee7-7b6c1f8307f9" />
<img width="1698" height="1007" alt="Screenshot 2026-06-20 215631" src="https://github.com/user-attachments/assets/95ec83ea-16c1-4bca-bcfa-e84a6aba1ea0" />


> These are early implementations meant to support learning and experimentation.
---

##  Planned Features

> Future development will progressively introduce more advanced rendering concepts.

-  Image-Based Lighting (IBL) with HDR
-  Post-processing effects (Bloom, FXAA, Tonemapping)
-  Asset hot-reloading

---

## External Libs

| Component              |                  Tool used                          |
|------------------------|-----------------------------------------------------|
| **Graphics API**       | OpenGL (Core Profile)                               |
| **GUI**                | [Dear ImGui](https://github.com/ocornut/imgui)      |
| **Windowing/Input**    | [GLFW](https://github.com/glfw/glfw)                |
| **Model Loading**      | [Assimp](https://github.com/assimp/assimp)          |
| **Math**               | [GLM](https://github.com/g-truc/glm)                |
| **Development IDE**    | Visual Studio 2022                                  |

---

## Building the Project

### Prerequisites
- Visual Studio 2022 (or later)
- CMake (optional if using VS solution directly)
- OpenGL-compatible GPU

### Build Instructions
1. Clone the repo:
   bash: git clone https://github.com/dadusthecoder/opengl2.git

2. Open the .sln file in Visual Studio 2022.
  - Make sure the following directories are linked properly using vcpkg or you can link them manully:
  - /Dependencies ( GLFW,GLEW,Assimp ,glm , imGui)
  ## resources
  - /res (Shaders, models) 

 3. Build and Run.
 

### Controls and UI
  - Use mouse and WASD keys to navigate the camera.
  - Use the ImGui panel to:
  - Change model position, rotation, scale
  - Toggle gravity
  - Change light position and color
  - Reload shaders or models in real-time

  
