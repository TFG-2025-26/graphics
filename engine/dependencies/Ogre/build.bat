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
:: Por tanto, estamos en [ProyectoRaiz]\dependencies\Ogre
:: 
:: 1) COMPILEDIR => dónde está el CMakeLists.txt principal de OGRE
set "COMPILEDIR=%~dp0src\ogre"

:: 2) BUILDDIR => carpeta donde generaremos la build (solución .sln, binarios, etc.)
::    quedará en [ProyectoRaiz]\dependencies\Ogre\build
set "BUILDDIR=%~dp0build"

:: 3) DLLFOLDERS => carpeta final para recopilar DLLs.
::    Queremos [ProyectoRaiz]\bin, así que subimos 2 niveles:
::    %~dp0 -> [ProyectoRaiz]\dependencies\Ogre
::       ..  -> [ProyectoRaiz]\dependencies
::       ..  -> [ProyectoRaiz]
::    y luego \bin => [ProyectoRaiz]\bin
set "DLLFOLDERS=%~dp0..\..\bin"

:: 4) Plataforma de compilación
set "PLATFORM=x64"

echo INICIANDO COMPILACION DE OGRE

:: -------------------------------------------
:: Crear la carpeta de build si no existe
:: -------------------------------------------
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
pushd "%BUILDDIR%"

:: =========================================
:: Configuración de CMake
:: =========================================
cmake -A %PLATFORM% ^
  -DOGRE_BUILD_DEPENDENCIES=OFF ^
  -DOGRE_BUILD_COMPONENT_BULLET=OFF ^
  -DOGRE_BUILD_COMPONENT_BITES=OFF ^
  -DOGRE_BUILD_PLUGIN_ASSIMP=OFF ^
  -DOGRE_BUILD_PLUGIN_DOT_SCENE=OFF ^
  -DOGRE_BUILD_SAMPLES=OFF ^
  -DOGRE_INSTALL_SAMPLES=OFF ^
  -DOGRE_BUILD_TOOLS=ON ^
  -DOGRE_INSTALL_TOOLS=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_D3D9=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_D3D11=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_GLES2=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_TINY=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_VULKAN=OFF ^
  -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=ON ^
  -DOGRE_BUILD_PLUGIN_BSP=OFF ^
  -DOGRE_BUILD_PLUGIN_PCZ=OFF ^
  -DOGRE_BUILD_PLUGIN_OCTREE=OFF ^
  "%COMPILEDIR%"

:: =========================================
:: Compilación con MSBuild
:: =========================================
msbuild "Ogre.sln" /p:configuration=Debug   /p:Platform=%PLATFORM%
msbuild "Ogre.sln" /p:configuration=Release /p:Platform=%PLATFORM%

popd

:: =========================================
:: Copia de las DLL generadas a bin
:: =========================================
pushd "%BUILDDIR%\bin"

:: DLLs de Debug
for %%i in ("Debug\*.dll") do (
    xcopy /y /s "%%~i" "%DLLFOLDERS%"
)

:: DLLs de Release
for %%j in ("Release\*.dll") do (
    xcopy /y /s "%%~j" "%DLLFOLDERS%"
)

:: plugins.cfg
if exist "Release\plugins.cfg" (
    xcopy /y /s "Release\plugins.cfg" "%DLLFOLDERS%"
)

popd

:: =========================================
:: Fin
:: =========================================
echo OGRE compilado con exito
ENDLOCAL
exit /b 0