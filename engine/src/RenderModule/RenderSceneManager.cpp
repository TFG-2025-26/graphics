#include "RenderSceneManager.h"

// ---- FLUX_RENDER ----
#include "RenderManager.h"
#include "RenderScene.h"
#include "UIManager.h"

// ------ OGRE ------
#include <OgreRoot.h>
#include <OgreNode.h>

//---FLUX_UTILS----
#include"FluxError.h"

flux_render::RenderSceneManager::RenderSceneManager(Ogre::Root* root)
{
	_root = root;
	_sceneManager = _root->createSceneManager();

	_currentScene = nullptr;
}

flux_render::RenderSceneManager::~RenderSceneManager()
{
	for (auto& e : _scenes) {
		RenderScene* scene = e.second;

		if (scene != nullptr) {
			Ogre::SceneNode* rootNode = scene->getSceneNode();

			// Primero borramos RenderObjects.
			delete scene;

			// Luego borramos el nodo raíz de la escena.
			if (rootNode != nullptr && _sceneManager != nullptr) {
				Ogre::SceneNode* parent = rootNode->getParentSceneNode();

				if (parent != nullptr) {
					parent->removeChild(rootNode);
				}

				_sceneManager->destroySceneNode(rootNode);
			}
		}
	}

	_scenes.clear();
	_currentScene = nullptr;

	if (_root != nullptr && _sceneManager != nullptr) {
		_root->destroySceneManager(_sceneManager);
	}

	_sceneManager = nullptr;
	_uiManager = nullptr;
}

flux_render::RenderScene* flux_render::RenderSceneManager::createScene(const std::string& sceneID)
{
	auto it = _scenes.find(sceneID);

	if (it == _scenes.cend()) {
		Ogre::SceneNode* sceneRootNode = _sceneManager->getRootSceneNode()->createChildSceneNode(sceneID);

		RenderScene* newScene = new RenderScene(sceneID, _sceneManager, sceneRootNode);

		_scenes.insert({ sceneID, newScene });

		if (_currentScene == nullptr) {
			_currentScene = newScene;
			sceneRootNode->setVisible(true);
		}
		else sceneRootNode->setVisible(false);

		return newScene;
	}

	else return it->second;
}

void flux_render::RenderSceneManager::destroyScene(const std::string& sceneID)
{
	auto it = _scenes.find(sceneID);

	if (it == _scenes.end()) {
		return;
	}

	RenderScene* scene = it->second;

	if (_currentScene == scene) {
		_currentScene = nullptr;
	}

	Ogre::SceneNode* rootNode = nullptr;

	if (scene != nullptr) {
		rootNode = scene->getSceneNode();

		// Primero RenderObjects.
		delete scene;
	}

	// Luego nodo raíz de la escena.
	if (rootNode != nullptr && _sceneManager != nullptr) {
		Ogre::SceneNode* parent = rootNode->getParentSceneNode();

		if (parent != nullptr) {
			parent->removeChild(rootNode);
		}

		_sceneManager->destroySceneNode(rootNode);
	}

	_scenes.erase(it);

	if (_uiManager != nullptr) {
		_uiManager->clearScene(sceneID);
	}
}

bool flux_render::RenderSceneManager::setCurrentScene(const std::string& sceneID)
{
	if (_currentScene != nullptr) _currentScene->getSceneNode()->setVisible(false);

	auto it = _scenes.find(sceneID);

	if (it != _scenes.cend()) {
		_currentScene = it->second;
		_currentScene->getSceneNode()->setVisible(true);
		return true;
	}

	else
	{
		//throw std::exception("Escena no existente");
		throwFluxError(false, "No se encontro la escena " + sceneID);
	}
}

void flux_render::RenderSceneManager::setUIManager(flux_render::UIManager* ui)
{
	if (ui != nullptr)
		_uiManager = ui;
}

flux_render::RenderScene* flux_render::RenderSceneManager::getCurrentScene() const
{
	if (_currentScene != nullptr) return _currentScene;
	else
	{
		throw std::exception("Escena no existente");
		//throwFluxError(, "No se encontro la animacion "+ sceneID);
	}
}

flux_render::RenderScene* flux_render::RenderSceneManager::getScene(const std::string& sceneID) const
{
	auto it = _scenes.find(sceneID);

	if (it != _scenes.cend()) return it->second;

	return nullptr;
}

Ogre::SceneManager* flux_render::RenderSceneManager::getOgreSceneManager() const
{
	return _sceneManager;
}

const std::unordered_map<std::string, flux_render::RenderScene*>& flux_render::RenderSceneManager::getAllScenes() const
{
	return _scenes;
}
