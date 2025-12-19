#include "DebugDrawer.h"
#include <btBulletDynamicsCommon.h>

#include <OgreSceneManager.h>
#include <OgreManualObject.h>
#include <OgreSceneNode.h>
#include <OgreRenderOperation.h>
#include <OgreColourValue.h>
#include <iostream>

struct DebugDrawer::Impl : public btIDebugDraw {
    Ogre::SceneManager* _sceneManager;
    Ogre::ManualObject* _lines;
    int _debugMode;

    Impl(Ogre::SceneManager* sceneManager)
        : _sceneManager(sceneManager), _debugMode(DBG_DrawWireframe) {
        _lines = sceneManager->createManualObject("PhysicsDebugDrawer");
        _lines->setDynamic(true);
        _sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(_lines);
    }

    ~Impl() {
        if (_lines) {
            _sceneManager->destroyManualObject(_lines);
            _lines = nullptr;
        }
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override {
        Ogre::ColourValue cv(color.x(), color.y(), color.z());
        _lines->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
        _lines->position(from.x(), from.y(), from.z());
        _lines->colour(cv);
        _lines->position(to.x(), to.y(), to.z());
        _lines->colour(cv);
        _lines->end();
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
        btScalar distance, int, const btVector3& color) override {
        btVector3 to = PointOnB + normalOnB * distance;
        drawLine(PointOnB, to, color);
    }

    void reportErrorWarning(const char* warningString) override {
        std::cerr << "[Bullet Warning] " << warningString << std::endl;
    }

    void draw3dText(const btVector3&, const char*) override {
        // No implementado
    }

    void setDebugMode(int debugMode) override {
        _debugMode = debugMode;
    }

    int getDebugMode() const override {
        return _debugMode;
    }

    void clear() {
        _lines->clear();
    }

    void flush() {
        // No necesario si se usa begin/end en cada frame
    }
};

// ---------------------------
// Métodos públicos del wrapper
// ---------------------------

DebugDrawer::DebugDrawer(Ogre::SceneManager* sceneManager) {
    _impl = new Impl(sceneManager);
}

DebugDrawer::~DebugDrawer() {
    delete _impl;
}

void DebugDrawer::clear()
{
    _impl->clear();
}

void DebugDrawer::flush() {
    _impl->flush();
}

void DebugDrawer::drawLine(const void* from, const void* to, const void* color) {
    _impl->drawLine(*reinterpret_cast<const btVector3*>(from),
        *reinterpret_cast<const btVector3*>(to),
        *reinterpret_cast<const btVector3*>(color));
}

void DebugDrawer::drawContactPoint(const void* PointOnB, const void* normalOnB,
    float distance, int lifeTime, const void* color) {
    _impl->drawContactPoint(*reinterpret_cast<const btVector3*>(PointOnB),
        *reinterpret_cast<const btVector3*>(normalOnB),
        distance, lifeTime,
        *reinterpret_cast<const btVector3*>(color));
}

void DebugDrawer::reportErrorWarning(const char* warningString) {
    _impl->reportErrorWarning(warningString);
}

void DebugDrawer::draw3dText(const void* location, const char* textString) {
    _impl->draw3dText(*reinterpret_cast<const btVector3*>(location), textString);
}

void DebugDrawer::setDebugMode(int debugMode) {
    _impl->setDebugMode(debugMode);
}

int DebugDrawer::getDebugMode() const {
    return _impl->getDebugMode();
}

void* DebugDrawer::getBtIDebugDraw() const {
    return static_cast<btIDebugDraw*>(_impl);
}
