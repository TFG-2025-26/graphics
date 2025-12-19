#include "CAudioSource.h"

// ----- FLUX_AUDIO -----
#include "AudioManager.h"

// ----- FLUX_EC -----
#include "CTransform.h"
#include "Entity.h"

// ----- FLUX_UTILS -----
#include "FluxError.h"

#include "ComponentArguments.h"

namespace flux_ec {
	CAudioSource::~CAudioSource() {
		stop();
		audioMngr = nullptr;
	}

	bool CAudioSource::init(flux_script::ComponentArguments* args) {
		setPath(args->getValueToString("Sound"));
		setSoundName(args->getValueToString("Name"));
		setVolume(args->getValueToFloat("Volume"));
		setLoop(args->getValueToBool("Loop"));
		setPlayOnStart(args->getValueToBool("PlayOnStart"));
		setIsMusic(args->getValueToBool("Music"));

		//Instancia al audioManager
		audioMngr = flux_audio::AudioManager::instance();

		//Creamos el sonido
		if (!createSound()) {
			throwFluxError(false, "Fallo al inicializar el componente AudioSource");
		}

		if (playOnStart)
			play();

		audioMngr->setChannelVolume(soundName, volume);

		return true;
	}

	void CAudioSource::update(float dt) {
		//Comprobamos si el sonido se esta reproduciendo o no
		if (isPlaying && audioMngr) {
			if (!audioMngr->isSoundPlaying(soundName)) {
				isPlaying = false;
			}
		}
	}

	bool CAudioSource::createSound() {
		if (!audioMngr->loadSound(soundPath, soundName, isMusic)) {
			throwFluxError(false, "Error al cargar un sonido con el modulo de audio");
		}

		//Si es un audio en bucle, seteamos su modo a loop
		if (loop)
			audioMngr->setSoundMode(soundName, flux_audio::FLUX_SOUND_MODE::IN2D_LOOP);
		else
			audioMngr->setSoundMode(soundName, flux_audio::FLUX_SOUND_MODE::IN2D);

		return true;
	}

	void CAudioSource::play() {
		if (!isPlaying) {
			audioMngr->playSound(soundName);
			isPlaying = true;
		}
	}

	void CAudioSource::pause() {
		if (isPlaying) {
			audioMngr->pauseSound(soundName);
			isPlaying = false;
		}
	}

	void CAudioSource::resume() {
		if (!isPlaying) {
			audioMngr->resumeSound(soundName);
			isPlaying = true;
		}
	}

	void CAudioSource::stop() {
		if (isPlaying) {
			audioMngr->stopSound(soundName);
			isPlaying = false;
		}
	}

	void CAudioSource::changeChannelVolume(float newVolume) {
		audioMngr->setChannelVolume(soundName, newVolume);
		setVolume(newVolume);
	}

	void CAudioSource::setVolume(float v) { volume = v; };
	void CAudioSource::setSoundName(std::string name) { soundName = name; }
	void CAudioSource::setPath(std::string path) { soundPath = path; };
	//void CAudioSource::setChannelGroupName(std::string groupName) { channelGroupName = groupName; }
	void CAudioSource::setLoop(bool l) { loop = l; }
	//void CAudioSource::setIs3D(bool is3d) { is3D = is3d; }
	void CAudioSource::setPlayOnStart(bool play) { playOnStart = play; }
	void CAudioSource::setIsMusic(bool m) { isMusic = m; };
}
