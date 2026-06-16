#include "Material.h"

#include "helpers/Logger.h"

namespace lgt {

std::vector<MaterialGPU>                      g_MaterialGPU;
std::unordered_map<std::string, MaterialBRDF> g_MaterialBRDF;

void UpateMaterial(uint32_t index, MaterialGPU data) {
    CORE_ERROR("FUNCTION NOT IMPLEMENTED");
}

} // namespace lgt