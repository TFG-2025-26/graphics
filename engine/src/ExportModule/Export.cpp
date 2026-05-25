#include "Export.h"
#include "FluxBenchmarkMetrics.h"

#include <iostream>
#include <chrono>
#include <list>
#include <thread>

#include <RenderManager.h>
#include <InputManager.h>
#include <PhysicsManager.h>
#include <AudioManager.h>
#include <ScriptManager.h>

#include <Entity.h>
#include <CTransform.h>
#include <CCamera.h>
#include <CLight.h>
#include <CMeshRenderer.h>
#include <CAnimator.h>
#include <CRigidBody.h>
#include <CCollider.h>
#include <CAudioSource.h>
#include <CUI.h>

#include <ComponentFactory.h>

#include "RigidBodyWrapper.h"

#include <Vector3.h>
#include <Vector4.h>
#include <FluxError.h>
#include <SceneManager.h>

#define FLUX_ENGINE_BENCHMARK 0

#if FLUX_ENGINE_BENCHMARK
static double g_sceneLoadMs = 0.0;
static double g_memoryAfterSceneLoadMB = 0.0;
#endif

bool _isInitialized = false;
bool _isRunning = false;

void flux_export::Export::registerEngineComponents()
{
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CTransform>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CMeshRenderer>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CCamera>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CLight>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CAnimator>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CCollider>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CRigidBody>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CAudioSource>();
    flux_ec::ComponentFactory::instance()->registerComponent<flux_ec::CUI>();
}

bool flux_export::Export::initEngine()
{
#ifdef _DEBUG
    game = LoadLibraryA("game_d.dll");
#else
    game = LoadLibraryA("game.dll");
#endif

    if (game == NULL) {
        _isInitialized = false;
        throwFluxError(false, "No se ha creado la .dll del juego");
        //std::cout << "no has creado la .dll del juego\n";
    }
    else {
        FARPROC loadSceneFunc = GetProcAddress(game, "loadScene");

        if (loadSceneFunc == NULL) {
            throwFluxError(false, "Fallo al localizar la carga de escenas en el juego.");
        }
        else {
            loadScene = reinterpret_cast<SceneFuncNoArgsBool>(loadSceneFunc);
        }

        FARPROC registerGameComponentsFunc = GetProcAddress(game, "registerGameComponents");

        if (registerGameComponentsFunc == NULL) {
            throwFluxError(false, "Fallo al localizar la carga de componentes del juego.");
        }
        else {
            registerGameComponents = reinterpret_cast<SceneFuncNoArgs>(registerGameComponentsFunc);
        }

        registerEngineComponents();
        registerGameComponents();

        // Crear los managers
        physicsMng = flux_physics::PhysicsManager::instance();
        managers.push_back(physicsMng);

        rdrMngr = flux_render::RenderManager::instance();
        managers.push_back(rdrMngr);

        inputMngr = flux_input::InputManager::instance();
        managers.push_back(inputMngr);

        audioMng = flux_audio::AudioManager::instance();
        managers.push_back(audioMng);

        scriptMngr = flux_script::ScriptManager::instance();
        managers.push_back(scriptMngr);

        sceneMngr = flux_utils::SceneManager::instance();
        managers.push_back(sceneMngr);


        // Inicializar todos los managers y busqueda de errores en la inicializacion
        // la comprobacion se hace en caso de que en el metodo de init de cada modulo se lance excepciones

        if (!physicsMng->init()) {
            throwFluxError(false, "Fallo al inicializar el modulo de fisicas.");
        }
        if (!rdrMngr->init()) {
            throwFluxError(false, "Fallo al inicializar el modulo de renderizado.");
        }
        if (!inputMngr->init()) {
            throwFluxError(false, "Fallo al inicializar el modulo de input.");
        }
        if (!audioMng->init()) {
            throwFluxError(false, "Fallo al inicializar el modulo de audio.");
        }
        if (!scriptMngr->init()) {
            throwFluxError(false, "Fallo al inicializar el modulo de scripting.");
        }
        if (!sceneMngr->init()) {
            throwFluxError(false, "Fallo al inicializar el gestor de escenas.");
        }

#if FLUX_ENGINE_BENCHMARK
        const auto sceneLoadStart = std::chrono::high_resolution_clock::now();
#endif

        if (!loadScene()) {
            throwFluxError(false, "Fallo al cargar los preparar los datos del juego.");
        }

#if FLUX_ENGINE_BENCHMARK
        const auto sceneLoadEnd = std::chrono::high_resolution_clock::now();
        g_sceneLoadMs = std::chrono::duration<double, std::milli>(sceneLoadEnd - sceneLoadStart).count();
        g_memoryAfterSceneLoadMB = FluxBenchmarkMetrics::getProcessMemoryMB();
        // Para medir rendimiento sin capar por VSync. Si se quiere medir experiencia de usuario, comentar esta línea.
        rdrMngr->setSync(false);
#endif

#ifdef _DEBUG
        rdrMngr->enablePhysicsDebugDraw(physicsMng->getDynamicsWorld(), true);

#else
        // ---debug--//
        rdrMngr->enablePhysicsDebugDraw(physicsMng->getDynamicsWorld(), false);
#endif

        // Se ha inicializado correctamente
        _isInitialized = true;
        return true;
    }
}



FLUX_API bool flux_export::Export::callRunEngine()
{
    if (_isInitialized) {
        return runEngine();
    }
    else {
        throwFluxError(false, "No se puede acceder al bucle principal si no se ha inicializado el motor correctamente.");
    }
}

bool flux_export::Export::runEngine() {

    _isRunning = true;

#if FLUX_ENGINE_BENCHMARK
    const std::string backendName =
        rdrMngr->getBackendAPI() == BackendAPI::Ogre ? "Ogre" : "DirectX 12";
    const std::string csvPath =
        rdrMngr->getBackendAPI() == BackendAPI::Ogre ? "metrics_engine_ogre.csv" : "metrics_engine_directx12.csv";

    FluxBenchmarkMetrics benchmark(
        backendName,
        g_sceneLoadMs,
        g_memoryAfterSceneLoadMB,
        csvPath
    );
#endif

    auto startTime = std::chrono::high_resolution_clock::now();
    while (_isRunning) {
        // Calcular frameTime (en segundos)
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - startTime;
        float frameTime = elapsedTime.count();
        startTime = currentTime;

#if FLUX_ENGINE_BENCHMARK
        benchmark.beginFrame();
#endif

        // Actualizar todos los managers (esto procesa eventos, incluidos los del giroscopio)
        for (auto& m : managers) {
            m->update(frameTime);
        }



        // Comprobar si se debe cerrar la ventana (por ejemplo, al pulsar ESC)
        if (inputMngr->windowShouldClose()) {
            _isRunning = false;
        }

        flux_utils::SceneManager::instance()->processPendingSceneChange();

#if FLUX_ENGINE_BENCHMARK
        if (benchmark.endFrame()) {
            _isRunning = false;
        }
#endif
    }

    return true;
}

bool flux_export::Export::stopEngine()
{
    flux_script::ScriptManager::instance()->shutdown();
    flux_utils::SceneManager::instance()->shutdown();
    flux_render::RenderManager::instance()->shutdown();
    flux_physics::PhysicsManager::instance()->shutdown();
    flux_input::InputManager::instance()->shutdown();
    flux_audio::AudioManager::instance()->shutdown();

    flux_script::ScriptManager::close();
    flux_utils::SceneManager::close();
    flux_render::RenderManager::close();
    flux_physics::PhysicsManager::close();
    flux_input::InputManager::close();
    flux_audio::AudioManager::close();

    if (game != NULL) FreeLibrary(game);
    else {
        throwFluxError(false, "No existe ningun juego a eliminar");
    }

    _isInitialized = false;
    _isRunning = false;

    return true;
}
