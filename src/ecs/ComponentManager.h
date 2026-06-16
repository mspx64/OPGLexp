#pragma once
#include "ComponentId.h"
#include "ComponentRegistry.h"
#include "Defines.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace lgt {

class IComponentPool {
public:
    virtual bool                             hasComponent(const EntityHandle& entity) const = 0;
    virtual const std::vector<EntityHandle>& getEntitiesWithComponent() const               = 0;
    virtual ~IComponentPool()                                                               = default;
};

template <typename T> class ComponentPool : public IComponentPool {
private:
    std::vector<T>                           m_Components;
    std::unordered_map<EntityHandle, size_t> m_EntityToIndex;
    std::vector<EntityHandle>                m_IndexToEntity;

public:
    void addComponent(const EntityHandle& entity, const T& component) {
        auto it = m_EntityToIndex.find(entity);
        if (it == m_EntityToIndex.end()) {
            size_t newIndex = m_Components.size();
            m_Components.push_back(component);
            m_EntityToIndex[entity] = newIndex;
            m_IndexToEntity.push_back(entity);
        } else {
            m_Components[it->second] = component;
        }
    }

    bool removeComponent(const EntityHandle& entity) {
        auto it = m_EntityToIndex.find(entity);
        if (it != m_EntityToIndex.end()) {
            size_t       indexToRemove = it->second;
            EntityHandle lastEntity    = m_IndexToEntity.back();

            m_Components[indexToRemove]    = std::move(m_Components.back());
            m_EntityToIndex[lastEntity]    = indexToRemove;
            m_IndexToEntity[indexToRemove] = lastEntity;

            m_Components.pop_back();
            m_IndexToEntity.pop_back();
            m_EntityToIndex.erase(it);
            return true;
        }
        return false;
    }

    T& getComponent(const EntityHandle& entity) {
        auto it = m_EntityToIndex.find(entity);
        LGT_ASSERT_MSG(!(it == m_EntityToIndex.end()), "[ComponentPool::getComponent] Entity not found in pool.");
        return m_Components[it->second];
    }

    bool hasComponent(const EntityHandle& entity) const override { return m_EntityToIndex.find(entity) != m_EntityToIndex.end(); }

    std::vector<T>& getAllComponents() { return m_Components; }

    const std::vector<EntityHandle>& getEntitiesWithComponent() const override { return m_IndexToEntity; }

    size_t getSize() const { return m_Components.size(); }
};

struct View {
public:
    bool                             hasComponentType(ComponentId typeId) const;
    bool                             hasEntity(const EntityHandle& entity) const;
    void                             addEntity(const EntityHandle& entity);
    bool                             removeEntity(EntityHandle entity);
    const std::vector<EntityHandle>& getEntities() const;

    template <typename T> void addComponent(const EntityHandle& entity, const T& component) {
        getOrCreateComponentPool<T>()->addComponent(entity, component);
    }

    template <typename T> bool removeComponent(const EntityHandle& entity) {
        auto it = m_ComponentPools.find(getComponentId<T>());
        LGT_ASSERT_MSG(!(it == m_ComponentPools.end()), "[Archetype::removeComponent] Component pool not found.\n");
        auto pool = std::static_pointer_cast<ComponentPool<T>>(it->second);
        return pool->removeComponent(entity);
    }

    template <typename T> T& getComponent(const EntityHandle& entity) const {
        auto it = m_ComponentPools.find(getComponentId<T>());
        LGT_ASSERT_MSG(!(it == m_ComponentPools.end()), "[Archetype::getComponent()] Component pool not found.");
        Ref<ComponentPool<T>> pool = std::static_pointer_cast<ComponentPool<T>>(it->second);
        LGT_ASSERT_MSG(pool->hasComponent(entity), "[Archetype::getComponent()] Entity does not have component.");
        return pool->getComponent(entity);
    }

    template <typename T> std::vector<T>& getComponentPoolData() {
        auto it = m_ComponentPools.find(getComponentId<T>());
        LGT_ASSERT_MSG(!(it == m_ComponentPools.end()), "[Archetype::getComponentPoolData()] Component pool  not found");
        return std::static_pointer_cast<ComponentPool<T>>(it->second)->getAllComponents();
    }

    template <typename T> bool entityHasComponent(const EntityHandle& entity) const {
        auto it = m_ComponentPools.find(getComponentId<T>());
        if (it != m_ComponentPools.end()) {
            return it->second->hasComponent(entity);
        }
        return false;
    }

private:
    std::unordered_map<ComponentId, std::shared_ptr<IComponentPool>> m_ComponentPools;
    std::vector<EntityHandle>                                        m_Entities;

    template <typename T> Ref<ComponentPool<T>> getOrCreateComponentPool() {
        ComponentId typeId = getComponentId<T>();
        auto        it     = m_ComponentPools.find(typeId);

        if (it == m_ComponentPools.end()) {
            auto newPool             = std::make_shared<ComponentPool<T>>();
            m_ComponentPools[typeId] = newPool;
            return newPool;
        }
        return std::static_pointer_cast<ComponentPool<T>>(it->second);
    }
};

class LGT_API ComponentManager {
private:
    std::unordered_map<EntityHandle, Signature> m_EntityToSignature;
    std::unordered_map<Signature, Ref<View>>    m_Archetypes;

    void      moveEntity(const Signature& oldSig, const Signature& newSig, const EntityHandle& entity);
    Ref<View> getOrCreateArchetype(const Signature& sig);

public:
    static size_t ComponentCount;

    bool                                                  removeAllComponents(const EntityHandle& entity);
    const Signature&                                      getEntitySignature(const EntityHandle& entity);
    std::unordered_map<Signature, std::shared_ptr<View>>& getArchetypes();
    std::shared_ptr<View>                                 getArchetype(const Signature& sig) const;

    template <typename... Components> const View& getView() const {
        Signature target;
        target.set(getComponentId<Components>()...);

        auto it = m_Archetypes.find(target);
        LGT_ASSERT_MSG(it == m_Archetypes.end(), "view not found");
        return it->second;
    }

    template <typename T> void addComponent(const EntityHandle& entity, const T& component) {

        if (!m_EntityToSignature.count(entity)) {
            m_EntityToSignature[entity] = Signature();
        }

        Signature   oldSig = m_EntityToSignature[entity];
        Signature   newSig = oldSig;
        ComponentId cid    = getComponentId<T>();
        newSig.set(cid);

        if (oldSig != newSig) {
            moveEntity(oldSig, newSig, entity);
        }
        getOrCreateArchetype(newSig)->addComponent<T>(entity, component);
        m_EntityToSignature[entity] = newSig;
    }

    template <typename T> bool removeComponent(const EntityHandle& entity) {

        if (!m_EntityToSignature.count(entity))
            return false;
        Signature   oldSig = m_EntityToSignature[entity];
        ComponentId cid    = getComponentId<T>();
        if (!oldSig.test(cid))
            return false;
        Signature newSig = oldSig;
        newSig.reset(cid);

        if (oldSig != newSig) {
            const auto arch = this->getArchetype(oldSig);
            if (arch)
                arch->removeComponent<T>(entity);
            moveEntity(oldSig, newSig, entity);
            m_EntityToSignature[entity] = newSig;
            return true;
        }
        return false;
    }

    template <typename T> T& getComponent(const EntityHandle& entity) {
        return this->getArchetype(m_EntityToSignature[entity])->getComponent<T>(entity);
    }

    template <typename T> bool hasComponent(const EntityHandle& entity) {
        if (!m_EntityToSignature.count(entity))
            return false;
        ComponentId cid = getComponentId<T>();
        return m_EntityToSignature[entity].test(cid);
    }
};

} // namespace lgt
