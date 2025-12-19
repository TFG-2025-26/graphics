#pragma once

#ifndef ANIMATOR_H_
#define ANIMATOR_H_

#include "Component.h"
#include <unordered_set>
#include <string>

#include "defs.h"

namespace flux_script {
	class ComponentArguments;
}

namespace flux_ec {
	class CAnimator : public Component {
	public:
		FLUX_API CAnimator() = default;

		bool init(flux_script::ComponentArguments* args) override;
		void update(float dt) override;
		FLUX_API bool addAnimation(const std::string& animationName);
		FLUX_API void deleteAnimation(const std::string& animationName);
		FLUX_API void setAnimationEnabled(const std::string& animationName, bool enabled);
		FLUX_API void setAnimationLoop(const std::string& animationName, bool loop);

		static ID getID() { return "ANIMATOR"; }
		uint8_t getType() const override { return ANIMATOR; }
	private:
		std::unordered_set<std::string> _animations;
	};
}
#endif