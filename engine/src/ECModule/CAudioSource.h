#pragma once

#ifndef AUDIO_SOURCE_H_
#define AUDIO_SOURCE_H_

#include "Component.h"
#include <string>

#include "defs.h"

namespace flux_audio {
    class AudioManager;
}

namespace flux_script {
    class ComponentArguments;
}

/// <summary>
/// Clase AudioSource que se trata de un componente que se adhiere a una entidad para que dicha entidad pueda reproducir un sonido,
/// con la ayuda del audioManager
/// </summary>

namespace flux_ec {
    class CTransform;
    class Entity;
    class CAudioSource : public flux_ec::Component {
    public:
        FLUX_API CAudioSource() = default;
        FLUX_API virtual ~CAudioSource();

        // Métodos heredados de flux_ec::Component
        bool init(flux_script::ComponentArguments* args) override;
        void update(float dt) override;

        //Metodo para crear audio
        bool createSound();

        //Metodo para reporducir el sonido asociado a este componete
        FLUX_API void play();

        //Metodo para pausar el sonido asociado a este componete
        FLUX_API void pause();

        //Metodo para reanudar el sonido asociado a este componente
        FLUX_API void resume();

        //Metodo para parar un sonido asociado a este componente y liberar su canal en FMOD
        FLUX_API void stop();

        //Metodo para cambiar el volumen del canal en FMOD
        FLUX_API void changeChannelVolume(float newVolume);

        //Setters
        void setSoundName(std::string name);
        void setPath(std::string path);
        //void setChannelGroupName(std::string groupName);
        void setVolume(float v);
        void setLoop(bool l);
        //void setIs3D(bool is3d);
        void setPlayOnStart(bool play);
        void setIsMusic(bool m);

        //Getters
        inline std::string getSoundName() { return soundName; };
        //inline std::string getChannelGroupName() { return channelGroupName; };
        inline float getVolume() { return volume; };
        inline bool getIsLoop() { return loop; };
        //inline bool getIs3D() { return is3D; };
        inline bool getPlayOnStart() { return playOnStart; };

        static ID getID() { return "AUDIO_SOURCE"; }
        uint8_t getType() const override { return AUDIO_SOURCE; }
    private:

        //Nombre de la ruta del archivo a reproducir
        std::string soundPath;

        //Nombre del sonido a reproducir -> Es el nombre propio del sonido, no el nombre del archivo
        std::string soundName;

        //Nombre del grupo del canales en el que reproducir el sonido
        //std::string channelGroupName;

        //Componente Transfrom de la entidad a la que adherimos el componente para generar audio 3D (si es que lo quisieramos)
        //flux_ec::CTransform* transform;

        // Instancia del audioManager
        flux_audio::AudioManager* audioMngr = nullptr;

        // Volumen del sonido
        float volume = 0.0f;

        // Indica si el audio es 3D o no
        bool is3D = false;

        // Indica si el audio se reproduce en bucle
        bool loop = false;

        // Indica si el audio se reproduce al inicio o no
        bool playOnStart = false;

        // Indica si es un sonido para musica
        bool isMusic = false;

        // Indica si el sonido se esta reproduciendo
        bool isPlaying = false;
    };
}
#endif  // AUDIO_SOURCE_H_
