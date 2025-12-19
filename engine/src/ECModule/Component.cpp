#include "Component.h"

flux_ec::Entity* flux_ec::Component::getOwner() const
{
    return _owner;
}

void flux_ec::Component::setOwner(flux_ec::Entity* owner) {
    _owner = owner;
}

flux_ec::Component::Component(Entity* owner)
{
    _owner = owner;
    _isActive = true;
}

bool flux_ec::Component::getActive()
{
    return _isActive;
}
void flux_ec::Component::setActive(bool b)
{
    _isActive = b;
}
