@echo off

:: ===========================================
:: Evita que se muestren los comandos
:: y habilita ámbito local + expansión retardada
:: ===========================================
SETLOCAL ENABLEDELAYEDEXPANSION

set "COMPILEDIR=%~dp0"

set "BUILDDIR=%~dp0build"

set "PLATFORM=x64"

echo INICIANDO COMPILACION DE LUA

:: -------------------------------------------
:: Crear la carpeta de build si no existe
:: -------------------------------------------
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
pushd "%BUILDDIR%"

:: -------------------------------------------
:: Invocar cmake para generar la solución
:: -------------------------------------------
cmake -A %PLATFORM% "%COMPILEDIR%"

:: =========================================
:: Compilación con MSBuild
:: =========================================
msbuild "LuaWrapper.sln" /p:configuration=Debug   /p:Platform=%PLATFORM%
msbuild "LuaWrapper.sln" /p:configuration=Release /p:Platform=%PLATFORM%

popd

:: =========================================
:: Fin
:: =========================================
echo LUA compilado con exito
ENDLOCAL
exit /b 0