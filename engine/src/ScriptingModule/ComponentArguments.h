#pragma once

#ifndef COMPONENT_ARGUMENTS_H_
#define COMPONENT_ARGUMENTS_H_

#include "Component.h"

#include "defs.h"

#include <unordered_map>

namespace flux_utils {
	enum shapeType;
	enum rigidBodyType;
	class Vector2;
	class Vector3;
	class Vector4;
}

namespace flux_script {
	class ScriptManager;

	class ComponentArguments
	{
		friend ScriptManager;
	public:
		FLUX_API ComponentArguments() = default;

		FLUX_API flux_utils::Vector2 getValueToVector2(const std::string& key);
		FLUX_API flux_utils::Vector3 getValueToVector3(const std::string& key);
		FLUX_API flux_utils::Vector4 getValueToVector4(const std::string& key);
		FLUX_API bool getValueToBool(const std::string& key);
		FLUX_API int getValueToInt(const std::string& key);
		FLUX_API float getValueToFloat(const std::string& key);

		FLUX_API std::string getValueToString(const std::string& key);

		FLUX_API flux_utils::shapeType getValueToShapeType(const std::string& key);
		FLUX_API flux_utils::rigidBodyType getValueToRBType(const std::string& key);

		FLUX_API void setArg(const std::string& key, const std::string& value);

		FLUX_API std::list<std::string> getValueToListString(const std::string& key);
		FLUX_API std::list<bool> getValueToListBool(const std::string& key);
	private:
		std::unordered_map<std::string, std::string> _args;
	};
};


#endif // COMPONENT_ARGUMENTS_H_