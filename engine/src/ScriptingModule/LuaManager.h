#pragma once

#ifndef LUA_MANAGER_H_
#define LUA_MANAGER_H_

#include "Manager.h"
#include "Singleton.h"

#include <string>

#include "defs.h"

struct lua_State;

namespace flux_script {
	class LuaManager : public flux_utils::Manager, 
		public flux_utils::Singleton<LuaManager> 
	{
	public:
		FLUX_API friend Singleton<LuaManager>;

		FLUX_API virtual ~LuaManager();

		FLUX_API bool init() override;
		FLUX_API void update(float dt) override;
		FLUX_API bool shutdown() override;

		template <class T>
		FLUX_API void registerClasses(const std::string& luaName);
		FLUX_API bool loadScript(const std::string& filename);

		FLUX_API lua_State* getLuaState();
	private:
		FLUX_API LuaManager();

		lua_State* _l;
	};
}


#endif // LUA_MANAGER_H_