#pragma once

#include <tuple>
#include <string>

#include "Components.h"
// forward declarations
class EntityManager;

using ComponentTuple = std::tuple<CSprite, CAnimation, CState, CTransform, CBoundingBox, CInput, CPhysics, CLifespan>;

class Entity {
private:
    friend class EntityManager;
    Entity(size_t id, const std::string &tag);      // private ctor, entities can only be created by EntityManager

    const size_t            m_id{0};
    const std::string       m_tag{"Default"};
    bool                    m_active{true};
    ComponentTuple          m_components;

public:

    void                    destroy();
    const size_t&           getId() const;
    const std::string&      getTag() const;
    bool                    isActive() const;



    // Component API
    template<typename T>
    inline bool hasComponent() const {
        return getComponent<T>().has;
    }


    template<typename T, typename... TArgs>
    inline T& addComponent(TArgs &&... mArgs) {
        auto &component = getComponent<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.has = true;
        return component;
    }


    /*template<typename T>
    inline bool removeComponent() const {
        return getComponent<T>().has = false;
    }*/
    template<typename T>
    inline bool removeComponent() {
        return getComponent<T>().has = false;
    }

    // This function is a non-const version intended for mutable objects.
    // It returns a reference to the component of type T from a container, assumed to be m_components.
    template<typename T>
    inline T& getComponent() {
        return std::get<T>(m_components);
    }

    // This function is a const version intended for const objects.
    // The const version guarantees that the calling object remains unmodified and can be used with const objects.
    template<typename T>
    inline const T& getComponent() const {
        return std::get<T>(m_components);
    }
};

