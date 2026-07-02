#pragma once
// include/RenderX/core/Window.hpp

#include "Defines.h"
#include <string>
#include <functional>

struct GLFWwindow;

namespace lgt {

struct WindowSpec {
    std::string title  = "RenderX";
    u32         width  = 1280;
    u32         height = 720;
    bool        vsync  = true;
};

// Resize callback: (width, height)
using ResizeCallback = std::function<void(u32, u32)>;

class Window {
public:
    explicit Window(const WindowSpec& spec);
    ~Window();

    LGT_NON_COPYABLE(Window);
    LGT_DEFAULT_MOVABLE(Window);

    void               PollEvents() const;
    void               SwapBuffers() const;
    [[nodiscard]] bool ShouldClose() const;

    void SetResizeCallback(ResizeCallback cb) { m_ResizeCb = std::move(cb); }

    [[nodiscard]] u32         GetWidth() const { return m_Spec.width; }
    [[nodiscard]] u32         GetHeight() const { return m_Spec.height; }
    [[nodiscard]] GLFWwindow* GetHandle() const { return m_Handle; }
    [[nodiscard]] float       GetAspect() const;

private:
    static void GlfwErrorCallback(int code, const char* desc);
    static void GlfwFramebufferSizeCallback(GLFWwindow* w, int width, int height);

    WindowSpec     m_Spec;
    GLFWwindow*    m_Handle = nullptr;
    ResizeCallback m_ResizeCb;
};

} // namespace lgt
