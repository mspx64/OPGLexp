// src/core/Application.cpp
#include "Application.h"

namespace lgt {

Application::Application(const WindowSpec& spec) {
    m_Window = std::make_unique<Window>(spec);
    m_Window->SetResizeCallback([this](u32 w, u32 h) { OnResize(w, h); });
    m_Input = std::make_unique<InputManager>(m_Window->GetHandle());
}

Application::~Application() {
    m_Input.reset();
    OnShutdown();
    m_Window.reset();
}

void Application::Run() {
    OnInit();

    while (!m_Window->ShouldClose() && m_Running) {
        m_Timer.Tick();
        m_Input->BeginFrame();
        m_Window->PollEvents();
        OnUpdate(static_cast<f32>(m_Timer.DeltaTime()));

        OnRender();

        m_Window->SwapBuffers();
    }
}

} // namespace lgt
