#pragma once

#include <cstdlib>

// ==================== Export/Import Macros ====================
#ifdef LGT_PLATFORM_WINDOWS
#ifdef LGT_BUILD_DLL
#define LGT_API __declspec(dllexport)
#else   
#define LGT_API __declspec(dllimport)
#endif
#else
#define LGT_API
#endif

// ==================== Debug Break ====================
#if defined(_MSC_VER)
#define LGT_DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define lgt_DEBUGBREAK() __builtin_trap()
#else
#define lgt_DEBUGBREAK() std::abort()
#endif

// ==================== Assert Macros ====================
#ifdef _DEBUG
#define LGT_ASSERT(expr)                                                        \
        {                                                                           \
            if (!(expr)) {                                                          \
                                 \
                LGT_DEBUGBREAK();                                                   \
            }                                                                       \
        }

#define LGT_ASSERT_MSG(expr, msg, ...)                                          \
        {                                                                           \
            if (!(expr)) {                                                           \
                LGT_DEBUGBREAK();                                                   \
            }                                                                       \
        }
#else
#define LGT_ASSERT(expr) (void)0
#define LGT_ASSERT_MSG(expr, msg, ...) (void)0
#endif

// ==================== Component Registration ====================
#define LGT_REGISTER_COMPONENT(Namespace, ComponentType)                                \
namespace Namespace {                                                               \
    static void move##ComponentType(                                                \
        EntityHandle e,                                                             \
        const std::shared_ptr<Archetype>& oldArch,                                  \
        const std::shared_ptr<Archetype>& newArch)                                  \
    {                                                                               \
        auto data = oldArch->getComponent<ComponentType>(e);                        \
        newArch->addComponent<ComponentType>(e, data);                              \
        oldArch->removeComponent<ComponentType>(e);                                 \
    }                                                                               \
                                                                                    \
    static void remove##ComponentType(                                              \
        EntityHandle e,                                                             \
        const std::shared_ptr<Archetype>& arch)                                     \
    {                                                                               \
        arch->removeComponent<ComponentType>(e);                                    \
    }                                                                               \
                                                                                    \
    struct ComponentType##Registrar {                                               \
        ComponentType##Registrar() {                                                \
            ComponentRegistry::registerMoveFunc<ComponentType>(move##ComponentType);     \
            ComponentRegistry::registerRemoveFunc<ComponentType>(remove##ComponentType); \
        }                                                                           \
    };                                                                              \
                                                                                    \
    static ComponentType##Registrar s_##ComponentType##Registrar;                   \
}

// ==================== Logging Macros ====================
#define LGT_LOG_INIT()         /* ::lgt::Log::Init() */
#define LGT_CORE_TRACE(...)    /*::lgt::Log::GetCoreLogger()->trace(__VA_ARGS__)*/
#define LGT_CORE_INFO(...)     /*::lgt::Log::GetCoreLogger()->info(__VA_ARGS__)*/
#define LGT_CORE_WARN(...)     /*::lgt::Log::GetCoreLogger()->warn(__VA_ARGS__)*/
#define LGT_CORE_ERROR(...)    /*::lgt::Log::GetCoreLogger()->error(__VA_ARGS__)*/
#define LGT_CORE_CRITICAL(...) /*::lgt::Log::GetCoreLogger()->critical(__VA_ARGS__)*/

#define LGT_TRACE(...)         /*::lgt::Log::GetClientLogger()->trace(__VA_ARGS__)*/
#define LGT_INFO(...)          /*::lgt::Log::GetClientLogger()->info(__VA_ARGS__)*/
#define LGT_WARN(...)          /*::lgt::Log::GetClientLogger()->warn(__VA_ARGS__)*/
#define LGT_ERROR(...)         /*::lgt::Log::GetClientLogger()->error(__VA_ARGS__)*/
#define LGT_CRITICAL(...)      /*::lgt::Log::GetClientLogger()->critical(__VA_ARGS__)*/
