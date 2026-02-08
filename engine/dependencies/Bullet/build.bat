@echo off

:: ===========================================
:: Evita que se muestren los comandos
:: y habilita ámbito local + expansión retardada
:: ===========================================
SETLOCAL ENABLEDELAYEDEXPANSION

:: -------------------------------------------
:: RUTAS
:: -------------------------------------------
:: %~dp0 => Directorio donde está el propio .bat
:: Por tanto, estamos en [ProyectoRaiz]\dependencies\SDL
:: 
:: 1) COMPILEDIR => dónde está el CMakeLists.txt principal de SDL
set "COMPILEDIR=%~dp0src\bullet3"

:: 2) BUILDDIR => carpeta donde generaremos la build (solución .sln, binarios, etc.)
::    quedará en [ProyectoRaiz]\dependencies\SDL\build
set "BUILDDIR=%~dp0build"

:: 3) DLLFOLDERS => carpeta final para recopilar DLLs.
::    Queremos [ProyectoRaiz]\bin, así que subimos 2 niveles:
::    %~dp0 -> [ProyectoRaiz]\dependencies\SDL
::       ..  -> [ProyectoRaiz]\dependencies
::       ..  -> [ProyectoRaiz]
::    y luego \bin => [ProyectoRaiz]\bin
set "DLLFOLDERS=%~dp0..\..\bin"

:: 4) Plataforma de compilación
set "PLATFORM=x64"

echo INICIANDO COMPILACION DE BULLET

:: -------------------------------------------
:: Crear la carpeta de build si no existe
:: -------------------------------------------
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
pushd "%BUILDDIR%"

:: =========================================
:: Configuración de CMake
:: =========================================
cmake -A %PLATFORM% ^
  -DBUILD_BULLET2_DEMOS=OFF ^
  -DBUILD_CPU_DEMOS=OFF ^
  -DBUILD_OPENGL3_DEMOS=OFF ^
  -DBUILD_UNIT_TESTS=OFF ^
  -DBUILD_EXTRAS=OFF ^
  -DBUILD_CONVEX_DECOMPOSITION_EXTRA=OFF ^
  -DBUILD_GIMPACTUTILS_EXTRA=OFF ^
  -DBUILD_HACD_EXTRA=OFF ^
  -DBUILD_INVERSE_DYNAMIC_EXTRA=OFF ^
  -DBUILDOBJ2SDF_EXTRA=OFF ^
  -DBULLET2_MULTITHREADING=OFF ^
  -DENABLE_VHACD=OFF ^
  -DBUILD_CLSOCKET=OFF ^
  -DBUILD_ENET=OFF ^
  -DBUILD_BULLET_ROBOTICS_EXTRA=OFF ^
  -DBUILD_BULLET_ROBOTICS_GUI_EXTRA=OFF ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON ^
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ^
  %COMPILEDIR%

:: =========================================
:: Compilación con MSBuild
:: =========================================
msbuild "BULLET_PHYSICS.sln" /p:configuration=Debug   /p:Platform=%PLATFORM%
msbuild "BULLET_PHYSICS.sln" /p:configuration=Release /p:Platform=%PLATFORM%

popd

:: =========================================
:: Fin
:: =========================================
echo Bullet compilado con exito
ENDLOCAL
exit /b 0