// src/core/Window.cpp
#include "Window.h"
#include "Helpers/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace lgt {

void Window::GlfwErrorCallback(int code, const char* desc) {
    CORE_ERROR("[GLFW] Error {}: {}", code, desc);
}

void Window::GlfwFramebufferSizeCallback(GLFWwindow* w, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (self) {
        self->m_Spec.width  = static_cast<u32>(width);
        self->m_Spec.height = static_cast<u32>(height);
        if (self->m_ResizeCb)
            self->m_ResizeCb(self->m_Spec.width, self->m_Spec.height);
    }
}

// ── Construction / destruction ────────────────────────────────────────────────x

Window::Window(const WindowSpec& spec)
    : m_Spec(spec) {
    glfwSetErrorCallback(GlfwErrorCallback);

    if (!glfwInit())
        throw std::runtime_error("[RenderX] Failed to initialise GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#if defined(LGT_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_Handle =
        glfwCreateWindow(static_cast<int>(spec.width), static_cast<int>(spec.height), spec.title.c_str(), nullptr, nullptr);

    if (!m_Handle) {
        glfwTerminate();
        throw std::runtime_error("[RenderX] Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_Handle, this);
    glfwSetFramebufferSizeCallback(m_Handle, GlfwFramebufferSizeCallback);

    glfwMakeContextCurrent(m_Handle);
    glfwSwapInterval(spec.vsync ? 1 : 0);

    // Load OpenGL function pointers via GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_Handle);
        glfwTerminate();
        throw std::runtime_error("[RenderX] Failed to load OpenGL (GLAD)");
    }

    CORE_INFO("Window created  {}x{}  GL {}.{}", spec.width, spec.height, GLVersion.major, GLVersion.minor);
}

Window::~Window() {
    if (m_Handle) {
        glfwDestroyWindow(m_Handle);
        glfwTerminate();
    }
}

void Window::PollEvents() const {
    glfwPollEvents();
}
void Window::SwapBuffers() const {
    glfwSwapBuffers(m_Handle);
}
bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Handle) != 0;
}

float Window::GetAspect() const {
    return m_Spec.height > 0 ? static_cast<float>(m_Spec.width) / static_cast<float>(m_Spec.height) : 1.f;
}

} // namespace lgt
