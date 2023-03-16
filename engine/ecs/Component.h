#pragma once
#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include "../core/Types.h"

namespace nebula {

using ComponentType = u8;
constexpr ComponentType MAX_COMPONENT_TYPES = 64;
constexpr ComponentType INVALID_COMPONENT_TYPE = 0xFF;

template <typename T>
struct ComponentID {
    static ComponentType id;
};

template <typename T>
ComponentType ComponentID<T>::id = INVALID_COMPONENT_TYPE;

template <typename T, ComponentType ID>
struct RegisteredComponent {
    struct Registrar {
        Registrar() {
            ComponentID<T>::id = ID;
        }
    };
    static Registrar registrar;
};

template <typename T, ComponentType ID>
typename RegisteredComponent<T, ID>::Registrar RegisteredComponent<T, ID>::registrar;

struct ComponentMetadata {
    std::string displayName;
    std::string category;
    std::string tooltip;
    bool visibleInEditor;
    bool canBeRemoved;

    ComponentMetadata()
        : visibleInEditor(true)
        , canBeRemoved(true) {}
};

struct ComponentDependency {
    ComponentType requiredType;
    bool optional;
};

class Component {
public:
    virtual ~Component() = default;
    virtual ComponentType getType() const = 0;

    void setEnabled(bool enabled) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    virtual ComponentMetadata getMetadata() const {
        ComponentMetadata meta;
        return meta;
    }

    virtual std::vector<ComponentDependency> getDependencies() const {
        return {};
    }

private:
    bool mEnabled = true;
};

class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void create(void* entity) = 0;
    virtual void destroy(void* entity) = 0;
    virtual void* get(void* entity) = 0;
    virtual void setComponentEnabled(void* entity, bool enabled) = 0;
    virtual bool isComponentEnabled(void* entity) = 0;
};

#define REGISTER_COMPONENT(T, ID) \
    template struct RegisteredComponent<T, ID>

}
