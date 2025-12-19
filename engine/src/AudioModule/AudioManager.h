#pragma once

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <iostream>

// --- FLUX_UTILS ---
#include "Manager.h"
#include "Singleton.h"

#include "defs.h"

using namespace std;

namespace FMOD {
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
    namespace Studio {
        class System;
        class EventInstance;
        class EventDescription;
        class Bank;
    }
};

typedef unsigned int FMOD_STUDIO_LOAD_BANK_FLAGS;
enum FMOD_RESULT;
struct FMOD_VECTOR;
enum FMOD_RESULT;

namespace flux_utils {
    class Vector3;
}

namespace flux_audio {
    enum FLUX_SOUND_MODE { IN2D, IN3D, IN2D_LOOP, IN3D_LOOP };

    class AudioManager : public flux_utils::Manager,
        public flux_utils::Singleton<AudioManager>
    {
    public:
        FLUX_API friend class flux_utils::Singleton<AudioManager>;

        bool init() override;
        void update(float dt) override;
        bool shutdown() override;

        //bool initModule();

        // Métodos del AudioManager
        FLUX_API bool loadSound(const string& soundPath, const string& soundName, bool isMusic);
        FLUX_API void unLoadSound(const string& soundName);
        FLUX_API void playSound(const string& soundName);
        FLUX_API void pauseSound(const string& soundName);
        FLUX_API void resumeSound(const string& soundName);
        FLUX_API void stopSound(const string& soundName);
        FLUX_API void setSoundMode(const string& soundName, const FLUX_SOUND_MODE& mode);
        FLUX_API void setChannelVolume(const string& soundName, float volume);
        FLUX_API void loadEvent(const string& eventName, const string& name, float value);
        FLUX_API void playEvent(const string& eventName);
        FLUX_API void setChannelGroupVolume(const string& channelGroupName, float volume);
        FLUX_API void stopChannel(int channelId);
        FLUX_API void stopEvent(const string& eventName, bool bImmediate = false);
        FLUX_API void getEventParameter(const string& eventName, const string& eventParameter, float parameter);
        FLUX_API void setEventParameter(const string& eventName, const string& eventParameter, float parameter);
        FLUX_API void stopAllChannels();
        FLUX_API void setChannel3dPosition(int nChannelId, const flux_utils::Vector3& vPosition);
        FLUX_API void setChannelVolume(int nChannelId, float fVolumedB);
        FLUX_API bool isPlaying(int nChannelId) const;
        FLUX_API bool isSoundPlaying(const std::string& soundName);
        FLUX_API bool isEventPlaying(const string& strEventName) const;
        FLUX_API float dbToVolume(float db);
        FLUX_API void setGlobalVolume(float fVolumedB);
        FLUX_API float volumeTodb(float volume);
        FMOD_VECTOR vector3ToFmodVector3(const flux_utils::Vector3& v);
        FMOD::Sound* getSound(const string& soundName);

        // Comprobación de errores
        bool errCheck(FMOD_RESULT res);

    private:
        AudioManager();
        // Puntero a la instancia de FMOD Studio (actualmente no inicializado)
        FMOD::Studio::System* studioSystem = nullptr;

        // Puntero a la instancia de FMOD Core
        FMOD::System* system = nullptr;

        // Variable para comprobar errores de FMOD
        FMOD_RESULT result;

        int nextChannelId = 0;
        bool isMuted = false;
        const int MAX_CHANNELS = 32;
        std::string relativePath = "../bin/assets/sounds/";

        // Mapa de sonidos y canales
        unordered_map<string, FMOD::Sound*> sounds;
        unordered_map<string, FMOD::Channel*> channels;
        // Nuevo mapa para almacenar si el sonido es de música (true) o no (false)
        unordered_map<string, bool> soundIsMusic;

        // Mapas para eventos
        unordered_map<string, FMOD::Studio::EventInstance*> events;
        unordered_map<string, FMOD::Studio::EventDescription*> eventDescriptions;

        // Mapas para grupos de canales
        unordered_map<string, FMOD::ChannelGroup*> channelGroups;

        // Grupos de canales predefinidos
        FMOD::ChannelGroup* master = nullptr, * ambient = nullptr, * noises = nullptr;
    };
}

#endif // AUDIO_MANAGER_H
