#include "Entity.h"

// ---- FLUX_EC ----
#include "Component.h"

// ----- STD -----
#include <iostream>


// ---- FLUX_UTILS ----
#include "FluxError.h"

flux_ec::Entity::Entity(int id, const std::string& name)
{
	_id = id;
	_name = name;
}

flux_ec::Entity::~Entity()
{
	for (auto& e : _components) {
		delete e.second;
	}

	_components.clear();
}

void flux_ec::Entity::update(float dt)
{
	for (auto& e : _components) {
		e.second->update(dt);
	}
}

void flux_ec::Entity::addComponent(Component* component)
{
	auto it = _components.find(component->getType());

	if (it == _components.cend()) _components.insert({ component->getType(), component });
	else {
		writeFluxError("El componente ya ha sido anadido a la entidad" + getName());
	}
}

void flux_ec::Entity::removeComponent(const uint8_t& componentId)
{
	auto it = _components.find(componentId);

	if (it != _components.cend()) _components.erase(it);
	else {
		writeFluxError("El componente no puede eliminar un componente inexistente " + componentId);
	}
}

void flux_ec::Entity::removeAllComponents() {

}

flux_ec::Component* flux_ec::Entity::getComponent(const uint8_t& componentId) const
{
	auto it = _components.find(componentId);

	if (it != _components.cend()) return it->second;
	else {
		throwFluxError(nullptr, "No se puede acceder a un Componente no existente");
	}
}

bool flux_ec::Entity::hasComponent(const uint8_t& componentId) const
{
	auto it = _components.find(componentId);

	if (it != _components.cend()) return true;
	else return false;
}

std::string flux_ec::Entity::getName() const
{
	return _name;
}
int flux_ec::Entity::getId() const  
{  
   return _id;  
}
void flux_ec::Entity::display()
{
	std::cout << "Entity(ID: " << _id << ", Name: " << _name << ")\n";
	std::cout << "Components:\n";

	for (const auto& pair : _components) {
		std::cout << " - Component ID: " << pair.first << "\n";
	}
}

FLUX_API void flux_ec::Entity::markToDelete() {

	_delete = false ;
}

FLUX_API bool flux_ec::Entity::getMarkToDelete()
{
	return _delete;
}

void flux_ec::Entity::setSceneID(const std::string& sceneID)
{
	_sceneID = sceneID;
}

std::string flux_ec::Entity::getSceneID() const
{
	return _sceneID;
}
