#pragma once

#ifndef RENDER_OBJECT_H_
#define RENDER_OBJECT_H_

#include <string>
#include <unordered_map>

namespace Ogre {
	class SceneManager;
	class SceneNode;
	class Entity;
	class AnimationState;
}

namespace flux_utils {
	class Vector3;
	class Vector4;
}

namespace flux_render {

	class RenderObject
	{
	public:
		RenderObject(const std::string& id, Ogre::SceneManager* mngr, 
			Ogre::SceneNode* parent = nullptr);
		virtual ~RenderObject();

		void setPosition(const flux_utils::Vector3& pos);
		void setOrientation(const flux_utils::Vector4& rot);
		void setScale(const flux_utils::Vector3& scale);

		flux_utils::Vector3 getPosition() const;
		flux_utils::Vector4 getOrientation() const;
		flux_utils::Vector3 getScale() const;

		void lookAt(const flux_utils::Vector3& lookAt);
		void setDirection(const flux_utils::Vector3& direction);

		bool addAnimation(const std::string& animationName);
		bool deleteAnimation(const std::string& animationName);
		bool setAnimationEnabled(const std::string& animationName, bool enabled);
		bool setAnimationLoop(const std::string& animationName, bool loop);
		void updateAnimations(float dt);

		void updateAnimation(const std::string& animationName, float dt);

		Ogre::SceneNode* getSceneNode() const;
		void setEntity(Ogre::Entity* entity);
	protected:
		std::string _id;
		Ogre::SceneNode* _node = nullptr;
		Ogre::Entity* _entity = nullptr;

		Ogre::SceneManager* _mngr;

		std::unordered_map<std::string, Ogre::AnimationState*> _animations;
	};
}
#endif // RENDER_OBJECT_H_


