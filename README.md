# 🌌 OPGLexp (OpenGL Experimental Renderer)

This project is an **experimental renderer** built with **C++ and OpenGL**, created as a personal learning journey into the world of modern graphics programming, game engine UI design, and real-time rendering pipelines.

The long-term goal is to evolve this project into a **Physically Based Renderer (PBR)** from scratch. For now, it serves as a sandbox for experimenting with rendering techniques, shaders, and scene management systems.

---

## 📸 Screenshots
*(Note: These are early implementations meant to support learning and experimentation)*
<img width="1918" height="1075" alt="Screenshot 2025-10-25 230854" src="https://github.com/user-attachments/assets/fef2ce93-eb4c-4744-adce-cf77586e2f7f" />
<img width="1750" height="1034" alt="image" src="https://github.com/user-attachments/assets/94e25bba-bb9b-4363-aee7-7b6c1f8307f9" />
<img width="1698" height="1007" alt="Screenshot 2026-06-20 215631" src="https://github.com/user-attachments/assets/95ec83ea-16c1-4bca-bcfa-e84a6aba1ea0" />

---

## ✨ Features

- **Modern Unity-Style Editor UI**: A sleek, dark-themed ImGui interface that heavily mimics the Unity Engine workflow. Includes large crisp fonts, precise color palettes, and structured component data.
- **Scene Hierarchy**: A full scene graph implementation allowing nested nodes.
- **Advanced Inspector**: View and edit components (Transform, Mesh Filter, Mesh Renderer) on a per-node basis. Allows real-time material swapping on specific meshes within complex models.
- **Asset Browser**: Drag-and-drop or browse models dynamically at runtime.
- **Integrated Console**: A custom ImGui console intercepting `spdlog` messages in real-time.
- **vcpkg + CMake Build System**: Completely automated dependency management via `vcpkg.json` and manifest mode, compiling with Clang and Ninja.

---

## 🔮 Planned Features
> Future development will progressively introduce more advanced rendering concepts.
- Image-Based Lighting (IBL) with HDR
- Post-processing effects (Bloom, FXAA, Tonemapping)
- Asset hot-reloading

---

## 🛠 External Libs
| Component              |                  Tool used                          |
|------------------------|-----------------------------------------------------|
| **Graphics API**       | OpenGL 4.6 (Core Profile)                           |
| **GUI**                | [Dear ImGui](https://github.com/ocornut/imgui) (Docking Branch) |
| **Windowing/Input**    | [GLFW](https://github.com/glfw/glfw)                |
| **Model Loading**      | [Assimp](https://github.com/assimp/assimp)          |
| **Math**               | [GLM](https://github.com/g-truc/glm)                |
| **Logging**            | [spdlog](https://github.com/gabime/spdlog)          |
| **Build System**       | CMake, Ninja, Clang-CL, PowerShell                  |
| **Package Manager**    | vcpkg (Manifest Mode)                               |

---

## 🚀 Building the Project

### Prerequisites
- **Visual Studio 2022** (with C++ Desktop Development tools)
- **CMake** & **Ninja**
- **LLVM / Clang** installed
- **vcpkg** (Ensure `VCPKG_ROOT` environment variable is set)

### Build Instructions
The project uses an automated PowerShell script to configure CMake and build the project via Ninja and Clang-CL.

1. Clone the repo:
   ```bash
   git clone https://github.com/dadusthecoder/OPGLexp.git
   cd OPGLexp
   ```

2. Run the build script:
   ```powershell
   .\build.ps1
   ```
   *Note: On the first run, vcpkg will automatically download and build all dependencies statically (Assimp, ImGui, spdlog, GLFW, etc.). This may take several minutes.*

3. Run the application:
   ```powershell
   .\build\OPGLexp.exe
   ```

---

## 🎮 Controls and UI
- **Camera Navigation**: Hold `Right Mouse Button` to lock the cursor and look around. Use `W A S D` to move. Press `ESC` to unlock the cursor.
- **Scene Interaction**: Select nodes in the **Scene Hierarchy**.
- **Inspector**: Modify node properties, translation, rotation, scale, and swap materials for distinct meshes on the selected node.
- **Asset Browser**: Drag models directly into the viewport or use the file browser UI to load them dynamically.
