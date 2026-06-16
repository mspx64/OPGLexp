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

} // namespace lgt