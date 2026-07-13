#include "ComponentId.h"
#include "ComponentManager.h"

namespace lgt {

ComponentId getUniqueComponentId() {
    static ComponentId uniqueid      = 0;
    ComponentManager::ComponentCount = uniqueid;
    return uniqueid++;
}

size_t getComponentTypeCount() {
    return ComponentManager::ComponentCount;
}
} // namespace lgt
