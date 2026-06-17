#include "Material.h"

#include "helpers/Logger.h"

namespace lgt {

GLuint                                        g_MaterialSSBO;
std::vector<MaterialGPU>                      g_MaterialGPU;
std::unordered_map<std::string, MaterialBRDF> g_MaterialBRDF;

void UpateMaterial(uint32_t index, MaterialGPU mat) {
    glNamedBufferSubData(g_MaterialSSBO, index * sizeof(MaterialGPU), sizeof(MaterialGPU), &mat);
}

} // namespace lgt