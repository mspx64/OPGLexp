#pragma once
#ifdef _WIN32
#undef APIENTRY
#endif
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define ENABLE_ASSERTIONS 1;

class Log {
public:
    static void                             Init();
    static void                             Shutdown();
    static std::shared_ptr<spdlog::logger>& Core();
    static void                             LogStatus(const std::string& msg);

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

#define CORE_LOG_INIT()         Log::Init();
#define CORE_TRACE(msg, ...)    Log::Core()->trace("[{}:{}] " msg, __func__, __LINE__, ##__VA_ARGS__)
#define CORE_INFO(msg, ...)     Log::Core()->info("[{}] " msg, __func__, ##__VA_ARGS__)
#define CORE_WARN(msg, ...)     Log::Core()->warn("[{}:{}] " msg, __func__, __LINE__, ##__VA_ARGS__)
#define CORE_ERROR(msg, ...)    Log::Core()->error("[{}:{}] " msg, __func__, __LINE__, ##__VA_ARGS__)
#define CORE_CRITICAL(msg, ...) Log::Core()->critical("[{}:{}] " msg, __func__, __LINE__, ##__VA_ARGS__)

#if defined(__clang__)
#define LGT_DEBUGBREAK() __builtin_trap()
#elif defined(__GNUC__)
#define LGT_DEBUGBREAK() __builtin_trap()
#elif defined(_MSC_VER)
#define DEBUGBREAK() __debugbreak()
#else
#include <cstdlib>
#define LGT_DEBUGBREAK() std::abort()
#endif

#ifdef ENABLE_ASSERTIONS
#define ASSERT(expr)                                                                                                             \
    {                                                                                                                            \
        if (!(expr)) {                                                                                                           \
            spdlog::error("Assertion Failed! Expr: {}", #expr);                                                                  \
            DEBUGBREAK();                                                                                                        \
        }                                                                                                                        \
    }

#define ASSERT_MSG(expr, msg, ...)                                                                                               \
    {                                                                                                                            \
        if (!(expr)) {                                                                                                           \
            spdlog::error("Assertion Failed: {} | Expr: "                                                                        \
                          "{}",                                                                                                  \
                          fmt::format(msg, ##__VA_ARGS__),                                                                       \
                          #expr);                                                                                                \
            DEBUGBREAK();                                                                                                        \
        }                                                                                                                        \
    }
#else
#define ASSERT(expr) (void)0
#define ASSERT_MSG(expr, msg, ...)
#endif