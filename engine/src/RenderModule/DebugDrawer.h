#pragma once
#ifndef DEBUG_DRAWER_H_
#define DEBUG_DRAWER_H_

namespace Ogre {
    class SceneManager;
    class ManualObject;
}

/// Wrapper del sistema de dibujo de físicas Bullet, sin exponer Bullet directamente.
class DebugDrawer {
public:
    DebugDrawer(Ogre::SceneManager* sceneManager);
    ~DebugDrawer();

    void clear();
    void flush();

    // Métodos proxy para debug
    void drawLine(const void* from, const void* to, const void* color);
    void drawContactPoint(const void* PointOnB, const void* normalOnB,
        float distance, int lifeTime, const void* color);
    void reportErrorWarning(const char* warningString);
    void draw3dText(const void* location, const char* textString);
    void setDebugMode(int debugMode);
    int getDebugMode() const;

    // Devuelve el puntero interno como btIDebugDraw*
    void* getBtIDebugDraw() const;

private:
    struct Impl;
    Impl* _impl;
};

#endif // DEBUG_DRAWER_H_
