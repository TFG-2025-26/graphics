#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H

#include <unordered_map>
#include <string>

#include "Singleton.h"

#include "defs.h"

namespace flux_ec {
	class Component;
	class Entity;

	class ComponentFactory : public flux_utils::Singleton<ComponentFactory>
	{
		friend Component;
		friend Singleton<ComponentFactory>;
	public:
		template<typename T>
		void registerComponent() {
			if (!componentExists(T::getID()))
				_currentComponents.emplace(T::getID(), &ComponentFactory::createComponentNoArgs<T>);
		};

		template<typename T, typename... Ts>
		Component* createComponent(Ts&&... args) {
			if (!componentExists(T::getID())) return nullptr;

			Component* c = new T(args...);

			return c;
		}

		FLUX_API Component* createComponentByName(const std::string& id);

		FLUX_API static ComponentFactory* getInstance();

	private:
		std::unordered_map<std::string, Component* (*)()> _currentComponents;

		FLUX_API ComponentFactory() = default;

		FLUX_API bool componentExists(const std::string& id);

		template<typename T>
		static Component* createComponentNoArgs() {
			return new T();
		}
	};
}

#endif // COMPONENT_FACTORY_H