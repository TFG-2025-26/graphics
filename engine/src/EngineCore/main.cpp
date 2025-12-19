#define _CRTDBG_MAP_ALLOC

#include <Export.h>

#include <crtdbg.h>
#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#define closeEngineByError(message) std::cerr << "Error de Flux Engine: " << message << "\n"; return EXIT_FAILURE
#else
#define closeEngineByError(message) return EXIT_FAILURE
#endif

#ifdef _DEBUG
int main() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(86161);
#else
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCadShow) {
#endif

    flux_export::Export exp;

    if (!exp.initEngine()) {
        exp.stopEngine();
        closeEngineByError("Fallos al inicializar el motor.");
    }

    if (!exp.callRunEngine()) {
        exp.stopEngine();
        closeEngineByError("Fallos al ejecutar el motor.");
    }

    if (!exp.stopEngine()) {
        closeEngineByError("Fallos al cerrar el motor");
    }

    return EXIT_SUCCESS;
}
