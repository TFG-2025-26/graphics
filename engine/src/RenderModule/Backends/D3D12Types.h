#pragma once

#ifndef D3D12_TYPES_H_
#define D3D12_TYPES_H_

#include <cstdint>
#include <string>

#include "Vector3.h"
#include "Vector4.h"

namespace flux_render {

	struct D3D12Renderable {
		std::string sceneID;
		std::string entityID;
		std::string meshName;

		flux_utils::Vector3 position = flux_utils::Vector3(0, 0, 0);
		flux_utils::Vector4 rotation = flux_utils::Vector4(0, 0, 0, 1);
		flux_utils::Vector3 scale = flux_utils::Vector3(1, 1, 1);

		float debugColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		bool hasMesh = false;
		bool visible = true;
	};

	struct D3D12LightData {
		std::string sceneID;
		std::string entityID;

		flux_utils::Vector3 position = flux_utils::Vector3(0, 0, 0);
		flux_utils::Vector4 rotation = flux_utils::Vector4(0, 0, 0, 1);
		flux_utils::Vector3 diffuseColor = flux_utils::Vector3(1, 1, 1);

		uint8_t lightType = 1; // 0 = point, 1 = directional, 2 = spotlight
		float intensity = 1.0f;
		bool valid = false;
	};

	struct D3D12CameraData {
		std::string sceneID;
		std::string entityID;

		flux_utils::Vector3 position = flux_utils::Vector3(0, 5, -15);
		flux_utils::Vector4 rotation = flux_utils::Vector4(0, 0, 0, 1);

		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
		float fovYDegrees = 60.0f;

		bool valid = false;
	};

}

#endif