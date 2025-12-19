#pragma once
#ifndef COLLISION_UTILS_H_
#define COLLISION_UTILS_H_

#include <string>
#include <unordered_map>
#include "../PhysicsModule/PhysicsManager.h"

namespace flux_physics {

    inline CollisionGroup getCollisionGroupFromName(const std::string& name) {
        static const std::unordered_map<std::string, CollisionGroup> map = {
            { "DEFAULT", COLLISION_DEFAULT },
            { "PLAYER", COLLISION_PLAYER },
            { "ENEMY", COLLISION_ENEMY },
            { "TRIGGER", COLLISION_TRIGGER },
            { "PROJECTILE", COLLISION_PROJECTILE }
        };

        auto it = map.find(name);
        if (it != map.end()) return it->second;
        return COLLISION_DEFAULT;
    }

    inline std::string getCollisionGroupName(CollisionGroup group) {
        switch (group) {
        case COLLISION_DEFAULT: return "DEFAULT";
        case COLLISION_PLAYER: return "PLAYER";
        case COLLISION_ENEMY: return "ENEMY";
        case COLLISION_TRIGGER: return "TRIGGER";
        case COLLISION_PROJECTILE: return "PROJECTILE";
        default: return "UNKNOWN";
        }
    }

}

#endif // COLLISION_UTILS_H_
