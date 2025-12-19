#pragma once

#ifndef ENTITY_H_
#define ENTITY_H_

#include "Component.h"

#include "defs.h"

// ------- STD -------
#include <string>
#include <map>

namespace flux_ec {
    class Component;
    class Entity {
    public:
        FLUX_API Entity(int id, const std::string& name);
        FLUX_API virtual ~Entity();

        FLUX_API void update(float dt);

        FLUX_API void addComponent(Component* component);
        FLUX_API void removeComponent(const uint8_t& componentId);
        FLUX_API void removeAllComponents();

        FLUX_API Component* getComponent(const uint8_t& componentId) const;
        FLUX_API bool hasComponent(const uint8_t& componentId) const;
        FLUX_API std::string getName() const;
        FLUX_API int getId() const;
        FLUX_API void display();
        FLUX_API void markToDelete();
        FLUX_API bool getMarkToDelete();
        FLUX_API std::size_t getComponentCount() const {
            return _components.size();
        }

        FLUX_API void setSceneID(const std::string& sceneID);
        FLUX_API std::string getSceneID() const;
    private:
        int _id;
        std::string _name;
        std::string _sceneID;
        std::map<uint8_t, Component*> _components;
        bool _delete = false;
        // std::vector<bool> _usedComponents; -> idea a futuro
    };
}

#endif ENTITY_H_