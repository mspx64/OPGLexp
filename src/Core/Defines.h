#pragma once
#include "UUID.h"
#include <bitset>
#include <memory>

namespace lgt {

template <typename T> using Scope = std::unique_ptr<T>;

template <typename T> using Ref = std::shared_ptr<T>;

// forward delecration
struct View;

// defines for ECS
using EntityHandle                 = uint32_t;
using ComponentId                  = uint32_t;
const size_t      MAX_ENTITIES     = 50000;
const ComponentId MAX_COMPONENTS   = 32;
const ComponentId ComponentIdError = static_cast<ComponentId>(-1);
using Signature                    = std::bitset<MAX_COMPONENTS>;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using f32 = float;
using f64 = double;

#define LGT_NON_COPYABLE(T)                                                                                                      \
    T(const T&)            = delete;                                                                                             \
    T& operator=(const T&) = delete

#define LGT_NON_MOVABLE(T)                                                                                                       \
    T(T&&)            = delete;                                                                                                  \
    T& operator=(T&&) = delete

#define LGT_DEFAULT_MOVABLE(T)                                                                                                   \
    T(T&&)            = default;                                                                                                 \
    T& operator=(T&&) = default

#define LGT_BIT(n) (1u << (n))

namespace Detail {
void AssertFail(const char* expr, const char* file, int line, const char* msg);
}

} // namespace lgt
