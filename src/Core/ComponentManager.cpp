#include "ComponentManager.h"
#include "ComponentRegistry.h"
#include "Core.h"

namespace lgt {

size_t ComponentManager::ComponentCount = 0;

// class  Archtypes
bool View::hasComponentType(ComponentId typeId) const {
    return m_ComponentPools.find(typeId) != m_ComponentPools.end();
}

void View::addEntity(const EntityHandle& entity) {
    if (!hasEntity(entity))
        m_Entities.push_back(entity);
}

bool View::removeEntity(EntityHandle entity) {
    if (hasEntity(entity)) {
        m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity), m_Entities.end());
        return true;
    }
    return false;
}

bool View::hasEntity(const EntityHandle& entity) const {
    return std::find(m_Entities.begin(), m_Entities.end(), entity) != m_Entities.end();
}

const std::vector<EntityHandle>& View::getEntities() const {
    return m_Entities;
}

// class ComponentManager

std::shared_ptr<View> ComponentManager::getOrCreateArchetype(const Signature& sig) {
    auto it = m_Archetypes.find(sig);
    if (it == m_Archetypes.end()) {
        auto newArchetype = std::make_shared<View>();
        m_Archetypes[sig] = newArchetype;
        return newArchetype;
    }
    return it->second;
}

void ComponentManager::moveEntity(const Signature& oldSig, const Signature& newSig, const EntityHandle& entity) {
    if (oldSig == newSig)
        return;
    const auto oldArch = getOrCreateArchetype(oldSig);
    const auto newArch = getOrCreateArchetype(newSig);

    for (ComponentId i = 0; i <= ComponentManager::ComponentCount; i++) {
        if (oldSig.test(i) && newSig.test(i)) {
            auto moveFunc = ComponentRegistry::getMovefunc(i);
            if (moveFunc)
                moveFunc(entity, oldArch, newArch);
        } else if (oldSig.test(i) && !newSig.test(i)) {
            auto removeFunc = ComponentRegistry::getRemovefunc(i);
            if (removeFunc)
                removeFunc(entity, oldArch);
        }
    }

    oldArch->removeEntity(entity);
    newArch->addEntity(entity);
}

bool ComponentManager::removeAllComponents(const EntityHandle& entity) {
    if (!m_EntityToSignature.count(entity))
        return false;

    Signature oldSig = m_EntityToSignature[entity];
    Signature emptySig;
    if (const auto oldArch = getOrCreateArchetype(oldSig)) {
        for (ComponentId i = 0; i <= ComponentManager::ComponentCount; i++) {
            if (oldSig.test(i) && i < ComponentManager::ComponentCount) {
                auto removeFunc = ComponentRegistry::getRemovefunc(i);
                if (removeFunc)
                    removeFunc(entity, oldArch);
            }
        }
    }

    getArchetype(oldSig)->removeEntity(entity);
    m_EntityToSignature[entity] = emptySig;
    return true;
}

const Signature& ComponentManager::getEntitySignature(const EntityHandle& entity) {
    return m_EntityToSignature[entity];
}

std::unordered_map<Signature, std::shared_ptr<View>>& ComponentManager::getArchetypes() {
    return m_Archetypes;
}

std::shared_ptr<View> ComponentManager::getArchetype(const Signature& sig) const {
    auto it = m_Archetypes.find(sig);
    return (it != m_Archetypes.end()) ? it->second : nullptr;
}

} // namespace lgt
