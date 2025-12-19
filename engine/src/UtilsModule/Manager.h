#pragma once

#ifndef MANAGER_H_
#define MANAGER_H_

#include "defs.h"

namespace flux_utils {
    class Manager {
    protected:
        bool isInitialized = false; 
    public:
        FLUX_API virtual ~Manager() = default;

        FLUX_API virtual bool init() = 0;
        FLUX_API virtual void update(float dt) = 0;
        FLUX_API virtual bool shutdown() = 0;
        FLUX_API virtual bool isInit() {
            return isInitialized;
        }
    };
}


#endif MANAGER_H_
