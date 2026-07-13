#include "Helpers/Logger.h"
#include "ErrorReporting.h"

void GLAPIENTRY glDebugOutput(
    GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131222)
        return;

    if (type == GL_DEBUG_TYPE_PERFORMANCE)
        return;

    std::string sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application";
        break;
    default:
        sourceStr = "Other";
        break;
    }

    std::string typeStr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        typeStr = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        typeStr = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        typeStr = "Pop Group";
        break;
    default:
        typeStr = "Other";
        break;
    }

    std::string formatMsg =
        fmt::format("[OpenGL Debug ({})] Message: {} -> Source: {}\n -> Type: {}", id, message, sourceStr, typeStr);

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        CORE_CRITICAL("{}", formatMsg);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        CORE_ERROR("{}", formatMsg);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        CORE_WARN("{}", formatMsg);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        CORE_TRACE("{}", formatMsg);
        break;
    }
}

void enableReportGlErrors() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    CORE_INFO("OpenGL Advanced Hardware Debug Context registered successfully.");
}
