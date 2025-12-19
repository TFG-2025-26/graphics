#include "AudioManager.h"

// -- FLUX_UTILS --
#include "Vector3.h"
#include "FluxError.h"
#include "checkML.h"

// ---- FMOD ----
#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include <cmath>
#include <unordered_map>

// Mapa global para almacenar los canales activos asociados a un identificador
static unordered_map<int, FMOD::Channel*> activeChannels;

namespace flux_audio {

    AudioManager::AudioManager() {
        result = FMOD_OK;
    }

    bool AudioManager::init() {
        // Inicialización de FMOD
        result = FMOD::System_Create(&system);
        if (!errCheck(result)) {
            throwFluxError(false, "Error al crear el sistema de audio.");
        }

        result = system->init(MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
        if (!errCheck(result)) {
            throwFluxError(false, "Error al inicializar el sistema de audio.");
        }

        result = system->getMasterChannelGroup(&master);
        if (!errCheck(result)) {
            throwFluxError(false, "No se pudo inicializar el canal master.");
        }

        result = system->createChannelGroup("ambient", &ambient);
        if (!errCheck(result)) {
            throwFluxError(false, "No se pudo inicializar el canal ambient.");
        }

        result = system->createChannelGroup("noises", &noises);
        if (!errCheck(result)) {
            throwFluxError(false, "No se pudo inicializar el canal noises.");
        }

        result = master->addGroup(ambient);
        if (!errCheck(result)) {
            throwFluxError(false, "No se pudo anadir el canal ambient al master");
        }

        result = master->addGroup(noises);
        if (!errCheck(result)) {
            throwFluxError(false, "No se pudo anadir el canal noises al master");
        }

        channelGroups.insert({ "ambient", ambient });
        channelGroups.insert({ "noises", noises });

        isInitialized = true;
        return true;
    }

    void AudioManager::update(float dt) {
        result = system->update();
    }

    void AudioManager::setGlobalVolume(float fVolumedB) {
        float volume = dbToVolume(fVolumedB);
        std::cout << "SetGlobalVolume: fVolumedB = " << fVolumedB
            << ", volume lineal = " << volume << std::endl;
        if (master) {
            master->setVolume(volume);
        }
    }

    bool AudioManager::shutdown() {
        if (isInitialized) {
            // Detener todos los canales activos
            stopAllChannels();

            // Liberar sonidos
            for (auto it = sounds.begin(); it != sounds.end(); ++it) {
                FMOD::Sound* s = it->second;
                if (s) {
                    result = s->release();
                    if (!errCheck(result)) {
                        throwFluxError(false, "No se pudo borrar un sonido correctamente.");
                    }
                }
            }
            sounds.clear();
            soundIsMusic.clear();

            // Liberar eventos
            for (auto it = events.begin(); it != events.end(); ++it) {
                if (it->second) {
                    result = it->second->release();
                    if (!errCheck(result)) {
                        throwFluxError(false, "No se pudo borrar un evento correctamente.");
                    }
                }
            }
            events.clear();
            eventDescriptions.clear();

            // Liberar grupos de canales
            for (auto it = channelGroups.begin(); it != channelGroups.end(); ++it) {
                if (it->second) {
                    result = it->second->release();
                    if (!errCheck(result)) {
                        throwFluxError(false, "No se pudo borrar un grupo de canales correctamente.");
                    }
                }
            }
            channelGroups.clear();

            // Cerrar y liberar el sistema de FMOD
            result = system->close();
            if (!errCheck(result)) {
                throwFluxError(false, "No se pudo cerrar el sistema correctamente.");
            }
            result = system->release();
            if (!errCheck(result)) {
                throwFluxError(false, "No se pudo liberar el sistema correctamente.");
            }

            isInitialized = false;

            return true;
        }
        else {
            throwFluxError(false, "No se puede cerrar un modulo que no esta inicializado.");
        }
    }

    FMOD_VECTOR AudioManager::vector3ToFmodVector3(const flux_utils::Vector3& v) {
        FMOD_VECTOR newVector;
        newVector.x = v.getX();
        newVector.y = v.getY();
        newVector.z = v.getZ();
        return newVector;
    }

    FMOD::Sound* AudioManager::getSound(const string& soundName) { return sounds[soundName]; }

    bool AudioManager::errCheck(FMOD_RESULT res) {
        if (res != FMOD_OK) {
            cout << "FMOD Error (" << FMOD_ErrorString(result) << "): " << endl;
            return false;
        }
        else return true;
    }

    bool AudioManager::loadSound(const string& soundPath, const string& soundName, bool isMusic) {
        // Si el sonido no existe lo cargamos
        if (!sounds.count(soundName)) {
            string newPath = relativePath + soundPath;
            FMOD::Sound* newSound = nullptr;

            result = system->createSound(newPath.c_str(), FMOD_DEFAULT, 0, &newSound);
            if (!errCheck(result)) {
                throwFluxError(false, "No se encontro el archivo de sonido " + soundPath + " para cargar");
            }


            sounds.insert({ soundName, newSound });

            // Reproducimos pausado solo para obtener el canal y configurarlo
            FMOD::Channel* channel = nullptr;
            result = system->playSound(newSound, nullptr, true, &channel);
            if (!errCheck(result)) {
                throwFluxError(false, "No se pudo reproducir el sonido " + soundName + " a traves de un canal");
            }

            //Si el sonido es una musica lo añadimos al grupo de sonido de ambiente, sino al de efectos de sonido
            if (isMusic)
                channel->setChannelGroup(ambient);
            else
                channel->setChannelGroup(noises);

            channels.insert({ soundName, channel });
        }
       
        return true;
    }

    void AudioManager::unLoadSound(const string& soundName) {
        if (sounds.count(soundName)) {
            FMOD::Sound* sound = sounds[soundName];
            result = sound->release();
            if (!errCheck(result))
                return;
            sounds.erase(soundName);
            soundIsMusic.erase(soundName);
        }
    }

    void AudioManager::playSound(const string& soundName) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se puede reproducir el sonido " + soundName + " porque no se ha cargado");
            return;
        }

        FMOD::Sound* sound = sounds[soundName];

        result = system->playSound(sound, 0, false, &channels[soundName]);
        if (!errCheck(result)) {
            writeFluxError("No se pudo reproducir el sonido " + soundName + " a traves de un canal");
            return;
        }
    }

    void AudioManager::pauseSound(const string& soundName) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se pudo pausar el sonido " + soundName);
            return;
        }
        if (channels[soundName])
            channels[soundName]->setPaused(true);
    }

    void AudioManager::resumeSound(const string& soundName) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se pudo reanudar el sonido " + soundName);
            return;
        }
        if (channels[soundName])
            channels[soundName]->setPaused(false);
    }

    void AudioManager::stopSound(const string& soundName) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se pudo parar el sonido " + soundName);
            return;
        }
        if (channels[soundName])
            channels[soundName]->stop();
    }

    void AudioManager::setSoundMode(const string& soundName, const FLUX_SOUND_MODE& mode) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se pudo cambiar el modo de sonido de" + soundName);
            return;
        }

        switch (mode) {
        case IN2D:
            sounds[soundName]->setMode(FMOD_2D);
            break;
        case IN2D_LOOP:
            sounds[soundName]->setMode(FMOD_2D | FMOD_LOOP_NORMAL);
            break;
        case IN3D:
            sounds[soundName]->setMode(FMOD_3D);
            break;
        case IN3D_LOOP:
            sounds[soundName]->setMode(FMOD_3D | FMOD_LOOP_NORMAL);
            break;
        }
    }

    void AudioManager::setChannelVolume(const string& soundName, float volume) {
        if (!sounds.count(soundName)) {
            writeFluxError("No se pudo cambiar el volumen del canal asignado al sonido " + soundName);
            return;
        }

        // Se establece un intervalo entre 0 y 1
        if (volume <= 1.0f && volume >= 0)
            if (channels[soundName])
                channels[soundName]->setVolume(volume);

        float newVolume = 0.0f;
        if (channels[soundName])
            channels[soundName]->getVolume(&newVolume);

        std::cout << "Volumen actual : " << newVolume << std::endl;
    }

    void AudioManager::loadEvent(const string& eventName, const string& name, float value) {
        if (events.count(eventName))
            return;

        FMOD::Studio::EventDescription* eventDesc = nullptr;
        FMOD::Studio::EventInstance* eventInst = nullptr;
        studioSystem->getEvent(eventName.c_str(), &eventDesc);
        if (eventDesc) {
            eventDesc->createInstance(&eventInst);
            eventInst->setParameterByName(name.c_str(), value);
            events.insert({ eventName, eventInst });
            eventDescriptions.insert({ eventName, eventDesc });
        }
    }

    void AudioManager::playEvent(const string& eventName) {
        if (!events.count(eventName))
            return;
        events[eventName]->start();
    }

    void AudioManager::setChannelGroupVolume(const string& channelGroupName, float volume) {
        if (!channelGroups.count(channelGroupName))
            return;
        FMOD::ChannelGroup* chG = channelGroups[channelGroupName];
        chG->setVolume(volume);
    }

    void AudioManager::stopChannel(int channelId) {
        if (activeChannels.find(channelId) != activeChannels.end()) {
            FMOD::Channel* channel = activeChannels[channelId];
            bool isPlaying = false;
            channel->isPlaying(&isPlaying);
            if (isPlaying)
                channel->stop();
            activeChannels.erase(channelId);
        }
    }

    void AudioManager::stopEvent(const string& eventName, bool bImmediate) {
        if (!events.count(eventName))
            return;
        FMOD_STUDIO_STOP_MODE mode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
        events[eventName]->stop(mode);
    }

    void AudioManager::getEventParameter(const string& eventName, const string& eventParameter, float parameter) {
        if (!events.count(eventName))
            return;
        float value = 0.0f;
        events[eventName]->getParameterByName(eventParameter.c_str(), &value);
        std::cout << "Valor del parámetro '" << eventParameter << "' del evento '" << eventName << "': " << value << std::endl;
    }

    void AudioManager::setEventParameter(const string& eventName, const string& eventParameter, float parameter) {
        if (!events.count(eventName))
            return;
        events[eventName]->setParameterByName(eventParameter.c_str(), parameter);
    }

    void AudioManager::stopAllChannels() {
        for (auto& ch : channels) {
            if (ch.second) {
                bool isPlaying = false;
                ch.second->isPlaying(&isPlaying);
                if (isPlaying)
                {
                    ch.second->stop();
                }
            }
        }
        channels.clear();
    }

    void AudioManager::setChannel3dPosition(int nChannelId, const flux_utils::Vector3& vPosition) {
        if (activeChannels.find(nChannelId) == activeChannels.end())
            return;
        FMOD::Channel* channel = activeChannels[nChannelId];
        FMOD_VECTOR pos = vector3ToFmodVector3(vPosition);
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        channel->set3DAttributes(&pos, &vel);
    }

    void AudioManager::setChannelVolume(int nChannelId, float fVolumedB) {
        if (activeChannels.find(nChannelId) == activeChannels.end())
            return;
        float volume = dbToVolume(fVolumedB);
        activeChannels[nChannelId]->setVolume(volume);
    }

    bool AudioManager::isPlaying(int nChannelId) const {
        auto it = activeChannels.find(nChannelId);
        if (it == activeChannels.end())
            return false;
        bool isPlaying = false;
        it->second->isPlaying(&isPlaying);
        return isPlaying;
    }

    FLUX_API bool AudioManager::isSoundPlaying(const std::string& soundName)
    {
        //Comprobamos que el canal asociado al soundName exista
        if (!channels.count(soundName) || !channels[soundName])
            return false;

        bool isPlaying = false;
        channels[soundName]->isPlaying(&isPlaying);
        return isPlaying;
    }

    bool AudioManager::isEventPlaying(const string& strEventName) const {
        if (!events.count(strEventName))
            return false;
        FMOD_STUDIO_PLAYBACK_STATE state;
        events.at(strEventName)->getPlaybackState(&state);
        return (state == FMOD_STUDIO_PLAYBACK_PLAYING);
    }

    float AudioManager::dbToVolume(float db) {
        return pow(10.0f, db / 20.0f);
    }

    float AudioManager::volumeTodb(float volume) {
        if (volume <= 0.0f)
            return -INFINITY;
        return 20.0f * log10(volume);
    }

} // namespace flux_audio
