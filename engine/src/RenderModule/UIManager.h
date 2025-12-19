#pragma once

#ifndef UI_MANAGER_H_
#define UI_MANAGER_H_

#include <Manager.h>
#include <unordered_map>
#include <string>

namespace Ogre {
	class Overlay;
	class OverlaySystem;
	class OverlayContainer;
	class TextAreaOverlayElement;
	class OverlayManager;
	class SceneManager;
}

namespace flux_ec {
	class CUI;
}

namespace flux_render {
	struct OverlayData {
		Ogre::Overlay* overlay;
		Ogre::OverlayContainer* panel;
		Ogre::TextAreaOverlayElement* text = nullptr;
	};

	class UIManager : public flux_utils::Manager
	{
	public:
		FLUX_API UIManager() = default;
		FLUX_API ~UIManager() override = default;

		bool init() override;
		void update(float dt) override;
		bool shutdown() override;

		bool registerComponent(flux_ec::CUI* comp);
		void setSceneManager(Ogre::SceneManager* sceneMngr);

		FLUX_API void setSceneActive(const std::string& sceneID);
		void clearScene(const std::string& sceneID); // opcional

		void updateText(std::string sceneID, flux_ec::CUI* ui);
		FLUX_API void deleteText(std::string sceneID, flux_ec::CUI* ui);

	private:
		bool fontExists(const std::string& fontName) const;

		Ogre::OverlaySystem* _overlaySystem = nullptr;
		Ogre::OverlayManager* _overlayMngr = nullptr;
		Ogre::SceneManager* _sceneMngr = nullptr;

		std::unordered_map<std::string, std::unordered_map<flux_ec::CUI*, OverlayData>> _sceneOverlays;

		int _elemIndex = 0;
	};
}

#endif // UI_MANAGER_H_