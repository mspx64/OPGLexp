#include "Material.h"

namespace lgt {

std::vector<MaterialGPU>                      g_MaterialGPU;
std::unordered_map<std::string, MaterialBRDF> g_MaterialBRDF;

} // namespace lgt