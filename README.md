# 🌌 OpenGL Experimental Renderer

This project is an **experimental renderer** built with **C++ and OpenGL**, created as a personal learning journey into the world of modern graphics programming and real-time rendering pipelines.

The long-term goal is to evolve this project into a **Physically Based Renderer (PBR)** from scratch. For now, it serves as a sandbox for experimenting with rendering techniques, shaders, and scene management systems.

---
## screenshots
<img width="1918" height="1075" alt="Screenshot 2025-10-25 230854" src="https://github.com/user-attachments/assets/fef2ce93-eb4c-4744-adce-cf77586e2f7f" />

<img width="1911" height="1037" alt="Screenshot 2026-06-13 010821" src="https://github.com/user-attachments/assets/4f7792a2-e99d-43f7-84ad-3d69897cab1b" />

<img width="1919" height="1079" alt="Screenshot 2026-06-15 002810" src="https://github.com/user-attachments/assets/9c890898-f675-4820-8227-f01c3f6e47c2" />

## 📸 Current Features (Experimental)

> These are early implementations meant to support learning and experimentation.

- 🎮 **Interactive Camera System** with configurable movement speed
- 💡 **Lighting Settings Panel**
  - Position control
  - RGB color picker
  - Auto-alignment with the camera
- 🎨 **Real-Time Shader Reloading**
  - Reload vertex/fragment shaders without restarting the app
- 📦 **Model Controls**
  - Load `.fbx` or `.obj` files
  - Position, Rotation (in degrees), and Scaling transforms
  - Reset buttons for convenience
- 🌌 **Environment Controls**
  - Background color configuration
- 🧭 **Camera Diagnostics Panel**
  - Real-time position and direction info
- 🧪 **Render Mode Switching**
  - Fill, Wireframe, Point modes
- ⚖️ **Gravity Toggle** (basic simulation toggle)
- 📈 **FPS Display** (updated in real time via ImGui)

---

## 🚀 Planned Features

> Future development will progressively introduce more advanced rendering concepts.

- ✅ Physically Based Rendering (PBR)
  - Albedo, Roughness, Metalness, Normal maps
- ✅ Image-Based Lighting (IBL) with HDR
- ✅ Shadow Mapping (Directional + Point lights)
- ✅ Material system with GUI-based editing
- ✅ Post-processing effects (Bloom, FXAA, Tonemapping)
- ✅ Scene hierarchy and editor
- ✅ Entity Component System (ECS) for simulation
- ✅ Asset hot-reloading

---

## 🧱 Tech Stack

| Component              |                  Tool used                          |
|------------------------|-----------------------------------------------------|
| **Graphics API**       | OpenGL (Core Profile)                               |
| **GUI**                | [Dear ImGui](https://github.com/ocornut/imgui)      |
| **Windowing/Input**    | [GLFW](https://github.com/glfw/glfw)                |
| **Model Loading**      | [Assimp](https://github.com/assimp/assimp)          |
| **Math**               | [GLM](https://github.com/g-truc/glm)                |
| **Development IDE**    | Visual Studio 2022                                  |

---

## 🛠️ Building the Project

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

  
