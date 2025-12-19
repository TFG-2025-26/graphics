#include "UIManager.h"

#include <OgreOverlayManager.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlaySystem.h>
#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h>
#include <OgreFont.h>

#include <OgreSceneManager.h>

#include "CUI.h"
#include "Vector2.h"
#include "Vector3.h"
#include "FluxError.h"

bool flux_render::UIManager::init() {
	_overlaySystem = new Ogre::OverlaySystem();
	_overlayMngr = Ogre::OverlayManager::getSingletonPtr();
	isInitialized = (_overlayMngr != nullptr);
	return isInitialized;
}

void flux_render::UIManager::update(float dt) {
	(void)dt;
}

bool flux_render::UIManager::shutdown() {
	for (auto& [sceneID, overlays] : _sceneOverlays) {
		for (auto& [comp, data] : overlays) {
			if (data.text) _overlayMngr->destroyOverlayElement(data.text);
			if (data.panel) _overlayMngr->destroyOverlayElement(data.panel);
			if (data.overlay) _overlayMngr->destroy(data.overlay);
		}
	}
	_sceneOverlays.clear();

	if (_sceneMngr && _overlaySystem)
		_sceneMngr->removeRenderQueueListener(_overlaySystem);

	delete _overlaySystem;
	_overlaySystem = nullptr;

	return true;
}

bool flux_render::UIManager::registerComponent(flux_ec::CUI* comp) {
	if (!isInitialized || !comp) 
		return false;

	const std::string& name = comp->getOverlayName();
	std::string sceneID = comp->getSceneID();
	try {

		OverlayData data;
		data.overlay = _overlayMngr->create(name + "_overlay" + "_" + std::to_string(_elemIndex));
		data.panel = static_cast<Ogre::OverlayContainer*>(
			_overlayMngr->createOverlayElement("Panel", name + "_panel" + "_" + std::to_string(_elemIndex)));

		data.panel->setMetricsMode(Ogre::GMM_PIXELS);
		data.panel->setPosition(comp->getPosition().getX(), comp->getPosition().getY());
		data.panel->setDimensions(comp->getSize().getX(), comp->getSize().getY());
		data.panel->setMaterialName(comp->getMaterial());
		data.overlay->add2D(data.panel);
		data.overlay->setZOrder(comp->getPosition().getZ());
		data.overlay->hide(); // Ocultamos por defecto, se activará con setSceneActive

		if (!comp->getText().empty()) {
			data.text = static_cast<Ogre::TextAreaOverlayElement*>(
				_overlayMngr->createOverlayElement("TextArea", name + "_text" + "_" + std::to_string(_elemIndex)));
			data.text->setMetricsMode(Ogre::GMM_PIXELS);
			data.text->setCaption(comp->getText());
			if (fontExists(comp->getFont()))
				data.text->setFontName(comp->getFont());
			else
				data.text->setFontName("BlueHighway");
			data.text->setCharHeight(comp->getCharHeight());
			auto color = comp->getColor();
			data.text->setColour(Ogre::ColourValue(color.getX() / 255.f, color.getY() / 255.f, color.getZ() / 255.f));
			data.panel->addChild(data.text);
		}

		_sceneOverlays[sceneID].insert({ comp, data });

		//Resteamos indice si hace falta
		_elemIndex++;
		if (_elemIndex >= 99)
			_elemIndex = 0;
	}
	catch (...) {
		throwFluxError(false, "Error a la hora de registrar el compenente Overlay " + name + " de Ogre");
	}
	
}

void flux_render::UIManager::setSceneActive(const std::string& sceneID) {
	for (auto& [sid, overlays] : _sceneOverlays) {
		for (auto& [comp, data] : overlays) {
			if (data.overlay) data.overlay->hide();
		}
	}
	auto it = _sceneOverlays.find(sceneID);
	if (it != _sceneOverlays.end()) {
		for (auto& [comp, data] : it->second) {
			if (data.overlay) data.overlay->show();
		}
	}
}

void flux_render::UIManager::clearScene(const std::string& sceneID) {
	auto it = _sceneOverlays.find(sceneID);
	if (it != _sceneOverlays.end()) {
		for (auto& [comp, data] : it->second) {
			if (data.text) _overlayMngr->destroyOverlayElement(data.text);
			if (data.panel) _overlayMngr->destroyOverlayElement(data.panel);
			if (data.overlay) _overlayMngr->destroy(data.overlay);
		}
		_sceneOverlays.erase(it);
	}
}

bool flux_render::UIManager::fontExists(const std::string& fontName) const {
	if (Ogre::FontManager::getSingleton().resourceExists(fontName)) {
		return true;
	}
	try {
		Ogre::FontPtr mFont = Ogre::FontManager::getSingleton().create(fontName, "General");
		mFont->setParameter("type", "truetype");
		mFont->setParameter("source", fontName);
		mFont->setParameter("size", "26");
		mFont->setParameter("resolution", "96");
		mFont->load();
		return true;
	}
	catch (...) {
		writeFluxError("No existe una fuente con dicho nombre, se pondrá una por defecto");
		return false;
	}
}

void flux_render::UIManager::setSceneManager(Ogre::SceneManager* sceneMngr) {
	_sceneMngr = sceneMngr;
	if (_sceneMngr && _overlaySystem) _sceneMngr->addRenderQueueListener(_overlaySystem);
}

void flux_render::UIManager::updateText(std::string sceneID, flux_ec::CUI* ui) {
	auto it = _sceneOverlays.find(sceneID);
	if (it != _sceneOverlays.end()) {
		auto it2 = it->second.find(ui);
		if (it2 != it->second.end()) {
			it2->second.text->setCaption(it2->first->getText());
		}
	}
}

void flux_render::UIManager::deleteText(std::string sceneID, flux_ec::CUI* ui) {
	auto it = _sceneOverlays.find(sceneID);
	if (it != _sceneOverlays.end()) {
		auto it2 = it->second.find(ui);
		if (it2 != it->second.end()) {
			_overlayMngr->destroyOverlayElement(it2->second.text);
			_overlayMngr->destroyOverlayElement(it2->second.panel);
			_overlayMngr->destroy(it2->second.overlay);
		}
		it->second.erase(it2);
	}
}