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
#define ECS_DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define lgt_DEBUGBREAK() __builtin_trap()
#else
#define lgt_DEBUGBREAK() std::abort()
#endif

// ==================== Assert Macros ====================
#ifdef _DEBUG
#define LGT_ASSERT(expr)                                                                                                         \
    {                                                                                                                            \
        if (!(expr)) {                                                                                                           \
                                                                                                                                 \
            ECS_DEBUGBREAK();                                                                                                    \
        }                                                                                                                        \
    }

#define LGT_ASSERT_MSG(expr, msg, ...)                                                                                           \
    {                                                                                                                            \
        if (!(expr)) {                                                                                                           \
            ECS_DEBUGBREAK();                                                                                                    \
        }                                                                                                                        \
    }
#else
#define LGT_ASSERT(expr)               (void)0
#define LGT_ASSERT_MSG(expr, msg, ...) (void)0
#endif

#define LGT_REGISTER_COMPONENT(Namespace, ComponentType)                                                                         \
    namespace Namespace {                                                                                                        \
    static void                                                                                                                  \
        move##ComponentType(EntityHandle e, const std::shared_ptr<View>& oldArch, const std::shared_ptr<View>& newArch) {        \
        auto data = oldArch->getComponent<ComponentType>(e);                                                                     \
        newArch->addComponent<ComponentType>(e, data);                                                                           \
        oldArch->removeComponent<ComponentType>(e);                                                                              \
    }                                                                                                                            \
                                                                                                                                 \
    static void remove##ComponentType(EntityHandle e, const std::shared_ptr<View>& arch) {                                       \
        arch->removeComponent<ComponentType>(e);                                                                                 \
    }                                                                                                                            \
                                                                                                                                 \
    struct ComponentType##Registrar {                                                                                            \
        ComponentType##Registrar() {                                                                                             \
            ComponentRegistry::registerMoveFunc<ComponentType>(move##ComponentType);                                             \
            ComponentRegistry::registerRemoveFunc<ComponentType>(remove##ComponentType);                                         \
        }                                                                                                                        \
    };                                                                                                                           \
                                                                                                                                 \
    static ComponentType##Registrar s_##ComponentType##Registrar;                                                                \
    }
