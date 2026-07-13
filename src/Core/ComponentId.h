#pragma once
#include "Defines.h"

namespace lgt {

ComponentId LGT_API getUniqueComponentId();
size_t              getComponentTypeCount();

template <typename T> inline ComponentId getComponentId() {
    static ComponentId typeId = getUniqueComponentId();
    return typeId;
}
} // namespace lgt
