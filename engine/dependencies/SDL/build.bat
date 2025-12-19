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
set "COMPILEDIR=%~dp0src\SDL"

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

echo INICIANDO COMPILACION DE SDL

:: -------------------------------------------
:: Crear la carpeta de build si no existe
:: -------------------------------------------
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
pushd "%BUILDDIR%"

:: =========================================
:: Configuración de CMake
:: =========================================
cmake -A %PLATFORM% %COMPILEDIR%

:: =========================================
:: Compilación con MSBuild
:: =========================================
msbuild "SDL2.sln" /p:configuration=Debug   /p:Platform=%PLATFORM%
msbuild "SDL2.sln" /p:configuration=Release /p:Platform=%PLATFORM%

:: =========================================
:: Copia de las DLL generadas a bin
:: =========================================

:: DLLs de Debug
xcopy /y /s .\Debug\SDL2d.dll "%DLLFOLDERS%"

:: DLLs de Release
xcopy /y /s .\Release\SDL2.dll "%DLLFOLDERS%"

popd

:: =========================================
:: Fin
:: =========================================
echo SDL compilado con exito
ENDLOCAL
exit /b 0