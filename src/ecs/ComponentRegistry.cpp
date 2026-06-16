
#include "ComponentRegistry.h"

namespace lgt {

std::unordered_map<ComponentId, ComponentRegistry::F_ForMoving>& ComponentRegistry::MoveFunc() {
    static std::unordered_map<ComponentId, F_ForMoving> map;
    return map;
}

std::unordered_map<ComponentId, ComponentRegistry::F_ForRemoving>& ComponentRegistry::RemoveFunc() {
    static std::unordered_map<ComponentId, F_ForRemoving> map;
    return map;
}

ComponentRegistry::F_ForMoving ComponentRegistry::getMovefunc(ComponentId id) {
    auto& map = MoveFunc();
    auto  it  = map.find(id);
    return (it != map.end()) ? it->second : nullptr;
}

ComponentRegistry::F_ForRemoving ComponentRegistry::getRemovefunc(ComponentId id) {
    auto& map = RemoveFunc();
    auto  it  = map.find(id);
    return (it != map.end()) ? it->second : nullptr;
}

} // namespace lgt
