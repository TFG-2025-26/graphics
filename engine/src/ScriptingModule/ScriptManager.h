#pragma once

#ifndef SCRIPT_MANAGER_H_
#define SCRIPT_MANAGER_H_

#include "Manager.h"
#include "Singleton.h"

#include "defs.h"

#include <Entity.h>

#include <vector>
#include <string>

#include <unordered_map>


struct lua_State;

namespace flux_script {
	class ComponentArguments;

	class ScriptManager : public flux_utils::Manager,
		public flux_utils::Singleton<ScriptManager>
	{
	public:
		FLUX_API friend class Singleton<ScriptManager>;
		FLUX_API virtual ~ScriptManager();

		FLUX_API bool init() override;
		FLUX_API void update(float dt) override;
		FLUX_API bool shutdown() override;

		FLUX_API bool loadScene(const std::string& sceneName);

		FLUX_API bool readScene(const std::string& sceneName);
		FLUX_API bool readScript(const std::string& scriptName);
		FLUX_API bool readPrefabs(const std::string& scriptName);
	private:
		FLUX_API ScriptManager();

		bool parsePrefabs(lua_State* l);
		bool parseScene(const std::string& sceneName, lua_State* l);
		bool parseEntities(lua_State* l);

	};
}

#endif // SCRIPT_MANAGER_H_