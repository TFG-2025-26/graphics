#pragma once

#ifndef MESH_RENDERER_H_
#define MESH_RENDERER_H_

#include "Component.h"

// ---- STD ----
#include <string>

#include "defs.h"

namespace flux_script {
	class ComponentArguments;
}

namespace flux_ec {
	class CMeshRenderer : public Component
	{
	public:
		FLUX_API CMeshRenderer() = default;
		FLUX_API virtual ~CMeshRenderer();

		FLUX_API bool init(flux_script::ComponentArguments* args) override;
		void update(float dt) override;

		static ID getID() { return "MESH"; }
		uint8_t getType() const override { return MESH; }
	private:
		std::string _meshName;
		std::string _materialName;

	private:
		std::string stripKnownMeshExtension(const std::string& meshName);
	};
}


#endif // MESH_RENDERER_H_