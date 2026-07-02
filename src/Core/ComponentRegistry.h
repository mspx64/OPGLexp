#pragma once
#include "ComponentId.h"
#include "Core.h"
#include "Defines.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace lgt {

struct View; // forward declaration

struct LGT_API ComponentRegistry {
    using F_ForMoving   = std::function<void(EntityHandle, const std::shared_ptr<View>&, const std::shared_ptr<View>&)>;
    using F_ForRemoving = std::function<void(EntityHandle, const std::shared_ptr<View>&)>;

    static std::unordered_map<ComponentId, F_ForMoving>&   MoveFunc();
    static std::unordered_map<ComponentId, F_ForRemoving>& RemoveFunc();

    template <typename T> static void registerMoveFunc(F_ForMoving func) {
        ComponentId typid = getComponentId<T>();
        MoveFunc()[typid] = std::move(func);
        // LGT_CORE_INFO("Component Rigestered  | ComponentId : {}",typid);
    }

    template <typename T> static void registerRemoveFunc(F_ForRemoving func) {
        RemoveFunc()[getComponentId<T>()] = std::move(func);
    }

    static F_ForMoving   getMovefunc(ComponentId id);
    static F_ForRemoving getRemovefunc(ComponentId id);
};

} // namespace lgt
