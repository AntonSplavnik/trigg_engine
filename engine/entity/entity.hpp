#ifndef IENTITY_HPP
#define IENTITY_HPP

#include <cstddef>
#include <vector>
#include <iostream>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <utility>

using EntityID = size_t;

/*
    COMPONENTS
*/
class IComponentStorage {
public:
    virtual ~IComponentStorage() {}
};

template<typename T>
class ComponentStorage final : public IComponentStorage {
public:
    template<typename... Args>
    void add(EntityID id, Args&&... args);

    void remove(EntityID id);

    T* get(EntityID id) const;

    std::vector<T>& getAll();
private:
    std::unordered_map<EntityID, size_t> entityToIndex;
    std::vector<T> data;
};

/*
    ENTITY
*/
class IEntity {
public:
    virtual ~IEntity() {}

    EntityID getId() const noexcept;
protected: // allow IEntity instantiation only from derived class
    IEntity() : id(++idCounter) {}
private:
    const EntityID id;
    inline static EntityID idCounter = 0;
};

/*
    REGISTRY
*/
class Registry final {
public:
    template<typename T, typename... Args>
    void emplace(const IEntity* e, Args&&... args);

    template<typename T>
    void remove(const IEntity* e);

    template<typename T>
    std::vector<T>& get();

    template<typename T, typename... Args>
    requires std::derived_from<T, IEntity>
    std::weak_ptr<T> createEntity(Args&&... args);

    void destroyEntity(const IEntity* e);

private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> componentsStorage;
    std::unordered_map<EntityID, std::shared_ptr<IEntity>> entitiesStorage;
};

/*
    COMPONENTS
*/
template<typename T>
template<typename... Args>
void ComponentStorage<T>::add(EntityID id, Args&&... args) {
    if (!entityToIndex.contains(id)) {
        data.emplace_back(T(std::forward<Args>(args)...));
        entityToIndex[id] = data.size() - 1;
    }
}

template<typename T>
void ComponentStorage<T>::remove(EntityID id) {
    if (auto it = entityToIndex.find(id); it != entityToIndex.end()) {
        data.erase(data.begin() + it->second);
        entityToIndex.erase(it);
    }
}

template<typename T>
T* ComponentStorage<T>::get(EntityID id) const {
    if (auto it = entityToIndex.find(id); it != entityToIndex.end())
        return data[it->second];

    return nullptr;
}

template<typename T>
std::vector<T>& ComponentStorage<T>::getAll() {
    return data;
}

/*
    ENTITY
*/
EntityID IEntity::getId() const noexcept {
    return id;
}

/*
    REGISTRY
*/
template<typename T, typename... Args>
requires std::derived_from<T, IEntity>
std::weak_ptr<T> Registry::createEntity(Args&&... args) {
    auto ent = std::make_shared<T>(std::forward<Args>(args)...);
    entitiesStorage[ent.get()->getId()] = ent;
    return ent;
}

void Registry::destroyEntity(const IEntity* e) {
    if (auto it = entitiesStorage.find(e->getId()); it != entitiesStorage.end()) {
        entitiesStorage.erase(it);
        // TODO: delete all components
    }
}

template<typename T>
void Registry::remove(const IEntity *e) {
    auto key = std::type_index(typeid(T));
    if (auto it = componentsStorage.find(key); it != componentsStorage.end()) {
        ComponentStorage<T> *c = static_cast<ComponentStorage<T>*>(it->second.get());
        c->remove(e->getId());
    }
}

template<typename T, typename... Args>
void Registry::emplace(const IEntity *e, Args&&... args) {
    auto key = std::type_index(typeid(T));
    if (const auto it = componentsStorage.find(key); it != componentsStorage.end()) {
        auto value = static_cast<ComponentStorage<T>*>(it->second.get());
        value->add(e->getId(), std::forward<Args>(args)...);
        return;
    }
    
    componentsStorage[key] = std::make_unique<ComponentStorage<T>>();
    auto value = static_cast<ComponentStorage<T>*>(componentsStorage[key].get());
    value->add(e->getId(), std::forward<Args>(args)...);
}

template<typename T>
std::vector<T>& Registry::get() {
    auto key = std::type_index(typeid(T));
    if (const auto it = componentsStorage.find(key); it != componentsStorage.end()) {
        ComponentStorage<T> *c = static_cast<ComponentStorage<T>*>(it->second.get());
        return c->getAll();
    }

    throw std::runtime_error("Component does not exist");
}

#endif