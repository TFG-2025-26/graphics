# Arquitectura gráfica modular para Flux Engine

Este repositorio recoge el trabajo realizado para el TFG **Desarrollo de una arquitectura gráfica de bajo nivel**, centrado en desacoplar el sistema de renderizado de Flux Engine y añadir una segunda vía de renderizado experimental basada en **DirectX 12**.

El proyecto parte de un motor académico existente basado en **Ogre**, **SDL**, **Bullet**, **FMOD** y **Lua**. Sobre esa base se ha trabajado en una capa de abstracción para que la lógica de escena no dependa directamente de Ogre, permitiendo ejecutar la misma escena mediante dos backends distintos:

- **Ogre**, como backend de referencia, con mallas, materiales, UI y animación usando sus sistemas nativos.
- **DirectX 12**, como backend experimental de bajo nivel, con reconstrucción de escena mediante geometría básica, cámara, profundidad, iluminación simple y sincronización propia.

El repositorio también incluye bancos de prueba independientes en **OpenGL**, **Vulkan** y **DirectX 12**, usados para comparar conceptos básicos de APIs gráficas y justificar la elección de DirectX 12 para la integración principal en Flux Engine.

---

## Estructura del repositorio

```text
.
├── engine/                  # Flux Engine modificado
│   ├── bin/                 # Ejecutables, DLLs y assets usados en ejecución
│   ├── dependencies/        # Dependencias externas y scripts de compilación
│   ├── projects/            # Proyectos de Visual Studio por módulo
│   ├── src/                 # Código fuente del motor
│   ├── FluxEngine.sln       # Solución principal del motor
│   └── build.bat            # Script de preparación/compilación de dependencias
│
├── test/                    # Bancos de prueba independientes por API
│   ├── OpenGL/
│   ├── Vulkan/
│   └── DirectX/
│
└── rhi/                     # Prototipo inicial de abstracción gráfica común
```

---

## Módulos principales del motor

Dentro de `engine/src/` el motor se organiza por módulos:

```text
AudioModule/       # Gestión de audio mediante FMOD
ECModule/          # Entidades y componentes del motor
EngineCore/        # Punto de entrada del ejecutable
ExportModule/      # Inicialización general, bucle principal y benchmark
InputModule/       # Entrada y eventos mediante SDL
PhysicsModule/     # Física mediante Bullet
RenderModule/      # Sistema de renderizado y backends Ogre/D3D12
ScriptingModule/   # Carga de escenas y datos mediante Lua
UtilsModule/       # Utilidades, vectores, singleton, errores, escenas
```

La parte más relevante del TFG se encuentra en:

```text
engine/src/RenderModule/
engine/src/RenderModule/Backends/
engine/src/ECModule/
engine/src/ExportModule/
```

---

## Backends de renderizado

El sistema de renderizado se ha adaptado para soportar dos caminos principales.

### Backend Ogre

El backend Ogre es la versión de referencia del motor. Usa las estructuras propias de Ogre para crear escenas, nodos, entidades, cámaras, luces, materiales y animaciones.

Archivos relevantes:

```text
engine/src/RenderModule/Backends/OgreBackend.*
engine/src/RenderModule/Backends/OgreSceneBackend.*
engine/src/RenderModule/RenderScene.*
engine/src/RenderModule/RenderObject.*
```

### Backend DirectX 12

El backend DirectX 12 es la implementación experimental añadida durante el TFG. No pretende igualar visualmente a Ogre, sino demostrar que los datos de escena del motor pueden reinterpretarse mediante una API de bajo nivel.

Actualmente reconstruye la escena mediante:

- geometría básica por entidad;
- `D3D12Renderable` como representación interna;
- cámara procedente del componente `CCamera`;
- luces procedentes de `CLight`;
- profundidad mediante depth buffer;
- color por entidad para diferenciar objetos;
- swapchain con soporte de tearing para métricas sin VSync;
- benchmark integrado para comparar Ogre y DirectX 12.

Archivos relevantes:

```text
engine/src/RenderModule/Backends/D3D12Backend.*
engine/src/RenderModule/Backends/D3D12SceneBackend.*
engine/src/RenderModule/Backends/D3D12Types.h
engine/src/RenderModule/Backends/IRenderBackend.h
engine/src/RenderModule/Backends/IRenderSceneBackend.h
```

> Nota: en esta versión del código, el backend DirectX 12 usa colores simples por entidad. Si en la memoria se habla de un formato `.fluxmat` como material propio, conviene asegurarse de que el cargador y los archivos `.fluxmat` estén incluidos en el repositorio o redactarlo como línea futura / material simple provisional.

---

## Selección de backend

El backend activo se selecciona actualmente en:

```text
engine/src/RenderModule/RenderManager.h
```

Busca la variable:

```cpp
BackendAPI _selectedAPI = BackendAPI::D3D12;
```

Valores disponibles:

```cpp
BackendAPI::Ogre   // Ejecuta el backend de referencia basado en Ogre
BackendAPI::D3D12  // Ejecuta el backend experimental DirectX 12
```

Ejemplo:

```cpp
// Backend por defecto del motor.
// Usar BackendAPI::Ogre para la versión de referencia.
// Usar BackendAPI::D3D12 para la reconstrucción experimental.
BackendAPI _selectedAPI = BackendAPI::D3D12;
```

También existe `RenderManager::setBackendAPI(BackendAPI api)`, pero debe llamarse antes de inicializar el render manager. En esta entrega se mantiene la selección en código para facilitar la revisión del TFG. Como mejora futura, podría moverse a un archivo externo de configuración.

---

## Benchmark del motor

El motor incluye una medición básica para comparar Ogre y DirectX 12 sobre la misma lógica de escena.

Archivos relevantes:

```text
engine/src/ExportModule/Export.cpp
engine/src/ExportModule/FluxBenchmarkMetrics.h
```

La macro de activación está en `Export.cpp`:

```cpp
#define FLUX_ENGINE_BENCHMARK 1
```

- `1`: activa benchmark. El motor mide, genera CSV y finaliza automáticamente.
- `0`: ejecución normal del motor.

Para entrega o demostración interactiva se recomienda dejar:

```cpp
#define FLUX_ENGINE_BENCHMARK 0
```

Para regenerar métricas:

```cpp
#define FLUX_ENGINE_BENCHMARK 1
```

El benchmark mide:

- tiempo de carga de escena;
- FPS medio;
- tiempo medio por frame;
- tiempo mínimo y máximo por frame;
- memoria tras cargar escena;
- memoria al finalizar la medición;
- número de frames medidos.

Los CSV se generan en el directorio de ejecución:

```text
metrics_engine_ogre.csv
metrics_engine_directx12.csv
```

Durante benchmark se llama a:

```cpp
rdrMngr->setSync(false);
```

para evitar que las mediciones queden limitadas por VSync. En DirectX 12 se ha añadido soporte para `DXGI_PRESENT_ALLOW_TEARING` cuando está disponible.

---

## Bancos de prueba de APIs gráficas

La carpeta `test/` contiene tres proyectos independientes utilizados para comparar una escena base en varias APIs:

```text
test/OpenGL/
test/Vulkan/
test/DirectX/
```

Estos bancos de prueba no son el resultado final del motor. Se usaron para estudiar diferencias entre OpenGL, Vulkan y DirectX 12 en una escena mínima, y para obtener métricas comparables en el capítulo de análisis.

Cada proyecto incluye su propio helper de métricas:

```text
test/OpenGL/src/BenchmarkMetrics.h
test/Vulkan/src/BenchmarkMetrics.h
test/DirectX/src/BenchmarkMetrics.h
```

CSV esperados:

```text
metrics_opengl.csv
metrics_vulkan.csv
metrics_directx12.csv
```

---

## Compilación y ejecución

### Recomendación importante

Este proyecto usa submódulos para varias dependencias. Es preferible clonar el repositorio con submódulos, por ejemplo:

```bash
git clone --recurse-submodules <url-del-repositorio>
```

Si ya se ha clonado sin submódulos:

```bash
git submodule update --init --recursive
```

También puede usarse GitHub Desktop, asegurándose de que los submódulos se descargan correctamente. Descargar el ZIP de GitHub puede dejar incompletas carpetas como `engine/dependencies/Ogre/src/ogre`, `Bullet/src/bullet3`, `SDL/src/SDL` o `lua/src/lua`.

### Preparar dependencias

Abrir **x64 Native Tools Command Prompt for VS** y ejecutar:

```bat
cd engine
build.bat
```

También existen scripts por dependencia:

```text
engine/dependencies/Ogre/build.bat
engine/dependencies/Bullet/build.bat
engine/dependencies/SDL/build.bat
engine/dependencies/lua/build.bat
```

### Compilar el motor

Abrir:

```text
engine/FluxEngine.sln
```

Configuración recomendada:

```text
x64 | Release
```

Para depuración puede usarse:

```text
x64 | Debug
```

El ejecutable principal se genera a través de `EngineCore` y carga la DLL del juego desde `engine/bin`.

---

## Assets y escenas

Los recursos se encuentran en:

```text
engine/bin/assets/
```

Estructura principal:

```text
animations/     # .skeleton de Ogre
fonts/          # fuentes para UI
materials/      # materiales .material de Ogre
meshes/         # mallas .mesh de Ogre
scenes/         # escenas Lua
sounds/         # sonidos usados por el juego
textures/       # texturas
```

Escenas principales:

```text
engine/bin/assets/scenes/scene.lua
engine/bin/assets/scenes/scene2.lua
engine/bin/assets/scenes/scene_mainMenu.lua
```

La escena se carga desde Lua mediante `ScriptingModule` y se traduce a entidades/componentes del motor. Después, cada backend decide cómo representar esos datos.

---

## Puntos relevantes de implementación

### Desacoplamiento de Ogre

La lógica de componentes ya no debería depender directamente de clases de Ogre para crear objetos de escena. En su lugar, se delega en interfaces comunes:

```text
IRenderBackend
IRenderSceneBackend
```

Esto permite que Ogre cree sus `SceneNode`/`Entity`, mientras que DirectX 12 guarda estructuras propias como `D3D12Renderable`, `D3D12LightData` y `D3D12CameraData`.

### DirectX 12 y VSync

Para medir correctamente el backend DirectX 12 se añadió soporte para tearing:

```text
DXGI_FEATURE_PRESENT_ALLOW_TEARING
DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
DXGI_PRESENT_ALLOW_TEARING
```

Si las métricas aparecen clavadas a 60 FPS, 144 FPS o 165 FPS, probablemente sigue activo algún tipo de sincronización con la pantalla.

### UI en DirectX 12

La UI está soportada por Ogre. En DirectX 12 no se ha implementado un sistema equivalente de interfaz, por lo que ciertos componentes UI deben tratarse como no disponibles o ignorarse en ese backend.

### Mallas y animaciones

Ogre usa `.mesh`, `.skeleton` y `.material`. DirectX 12 no carga todavía esos formatos. La reconstrucción actual usa geometría básica por entidad, por lo que no debe interpretarse como equivalencia visual completa.

---

## Limitaciones conocidas

- DirectX 12 no carga mallas reales `.mesh`.
- DirectX 12 no soporta animación esquelética `.skeleton`.
- La UI no está implementada en el backend DirectX 12.
- Los materiales en DirectX 12 son simples/provisionales en esta versión.
- La selección de backend se hace en código, no mediante archivo externo.
- La comparación Ogre vs DirectX 12 no es un benchmark absoluto, ya que Ogre dibuja la escena completa y DirectX 12 una reconstrucción simplificada.

---

## Ideas de ampliación futura

- Mover la selección de backend a un archivo de configuración.
- Crear un formato propio de malla, por ejemplo `.fluxmesh`.
- Añadir carga de texturas y materiales completos en DirectX 12.
- Implementar soporte de UI en DirectX 12.
- Añadir animación esquelética en el backend propio.
- Integrar Vulkan como tercer backend real dentro de Flux Engine.
- Convertir el prototipo `rhi/` en una capa común más formal.

---

## Resumen para entrevistas

Este repositorio demuestra el trabajo de modularización de un motor existente para separar la lógica de escena del backend de renderizado. La parte más relevante es la integración de un backend DirectX 12 experimental dentro de Flux Engine, reutilizando los datos de escena cargados desde Lua y reconstruyéndolos mediante estructuras propias de bajo nivel.

El valor principal del trabajo no es competir visualmente con Ogre, sino mostrar el proceso de extraer dependencias de un motor de alto nivel y preparar una arquitectura capaz de soportar varios backends gráficos.
