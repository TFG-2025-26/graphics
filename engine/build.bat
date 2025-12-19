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
:: 1) COMPILEDIR_OGRE => Directorio del build.bat de OGRE
set "COMPILEDIR_OGRE=%~dp0dependencies\Ogre"

:: 2) COMPILEDIR_BULLET => Directorio del build.bat de Bullet
set "COMPILEDIR_BULLET=%~dp0dependencies\Bullet"

:: 3) COMPILEDIR_SDL => Directorio del build.bat de SDL
set "COMPILEDIR_SDL=%~dp0dependencies\SDL"

:: 4) COMPILEDIR_LUA => Directorio del build.bat de lua
set "COMPILEDIR_LUA=%~dp0dependencies\lua"

:: 4) Plataforma de compilación
set "PLATFORM=x64"

:: -------------------------------------------
:: EJECUCIÓN DE LOS BATCH
:: -------------------------------------------

echo Ejecutando Ogre...
call "%COMPILEDIR_OGRE%\build.bat"

echo Ejecutando Bullet...
call "%COMPILEDIR_BULLET%\build.bat"

echo Ejecutando SDL...
call "%COMPILEDIR_SDL%\build.bat"

echo Ejecutando Lua...
call "%COMPILEDIR_LUA%\build.bat"

echo Todos los procesos han finalizado.
ENDLOCAL
exit /b 0