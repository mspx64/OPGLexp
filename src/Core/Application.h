#pragma once

#include "Window.h"
#include "Timer.h"
#include "InputManager.h"
#include <memory>
#include <stdexcept>

namespace lgt {

class Application {
public:
    explicit Application(const WindowSpec& spec);
    virtual ~Application();

    LGT_NON_COPYABLE(Application);

    // Main loop – blocks until the window closes
    void Run();

    // ── Override in derived class ─────────────────────────────────────────────
    virtual void OnInit()         = 0;
    virtual void OnUpdate(f32 dt) = 0;
    virtual void OnRender()       = 0;
    virtual void OnShutdown() {}
    virtual void OnResize(u32 w, u32 h) {
        (void)w;
        (void)h;
    }
    virtual void OnImGui() {} // called between Begin/End frame when ImGui enabled

    // ── Accessors for derived classes ─────────────────────────────────────────
    [[nodiscard]] Window&       GetWindow() { return *m_Window; }
    [[nodiscard]] Timer&        GetTimer() { return m_Timer; }
    [[nodiscard]] InputManager& GetInput() { return *m_Input; }

    void Quit() { m_Running = false; }

protected:
    std::unique_ptr<Window>       m_Window;
    Timer                         m_Timer;
    std::unique_ptr<InputManager> m_Input;
    bool                          m_Running = true;
};

} // namespace lgt

#define LGT_MAIN(AppClass)                                                                                                       \
    int main() {                                                                                                                 \
        Rx::Log::Init(#AppClass);                                                                                                \
        try {                                                                                                                    \
            AppClass app;                                                                                                        \
            app.Run();                                                                                                           \
        } catch (const std::exception& e) {                                                                                      \
            RX_CORE_CRITICAL("Fatal: {}", e.what());                                                                             \
            return 1;                                                                                                            \
        }                                                                                                                        \
        return 0;                                                                                                                \
    }
