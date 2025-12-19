#include "ComponentFactory.h"
#include "Component.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"

flux_ec::ComponentFactory* flux_ec::ComponentFactory::getInstance() {
	return static_cast<ComponentFactory*>(instance());
}

bool flux_ec::ComponentFactory::componentExists(const std::string& id) {
	return _currentComponents.count(id);
}

flux_ec::Component* flux_ec::ComponentFactory::createComponentByName(const std::string& id) {
	auto it = _currentComponents.find(id);
	if (it == _currentComponents.cend())
		//throwFluxError(nullptr, "No se pudo encontrar el componente a crear.");
		return nullptr;

	Component* c = it->second();

	return c;
}