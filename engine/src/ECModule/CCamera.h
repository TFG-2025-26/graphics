#pragma once

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Component.h"

namespace flux_script {
	class ComponentArguments;
}

namespace flux_ec {
	class CCamera : public Component
	{
	public:
		FLUX_API CCamera() = default;
		FLUX_API virtual ~CCamera() {}

		virtual bool init(flux_script::ComponentArguments* args) override;
		virtual void update(float dt) override;

		int8_t getNearClip() const;
		int16_t getFarClip() const;

		void setNearClip(int8_t nearDist);
		void setFarClip(int16_t farDist);

		static ID getID() { return "CAMERA"; }
		uint8_t getType() const override { return CAMERA; }

		bool isPendingCreation() const;
		bool createCamera();
	private:

		int8_t _nearDist = 0;
		int16_t _farDist = 0;

		bool _pendingCreation = false;
	};
}

#endif // CAMERA_H_