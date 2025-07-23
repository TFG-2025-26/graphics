#include <Windows.h>

#include "DX12App.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// Registrar clase de ventana
	const wchar_t CLASS_NAME[] = L"DX12WindowClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	SetProcessDPIAware();

	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	// Crear ventana
	HWND hwnd = CreateWindowEx(
		0,							// Estilo extendido
		CLASS_NAME,					// Nombre de clase
		L"DirectX 12",		// Título de ventana
		WS_OVERLAPPEDWINDOW,		// Estilo de ventana

		CW_USEDEFAULT, CW_USEDEFAULT, width, height, // Posición y tamaño

		nullptr,					// Ventana padre
		nullptr,					// Menú
		hInstance,					// Instancia
		nullptr						// Datos adicionales
	);

	if (hwnd == nullptr) return 0;

	ShowWindow(hwnd, nCmdShow);

	DX12App app;
	app.init(hwnd, width, height);

	// Bucle de mensajes
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			app.render();
		}
	}

	app.destroy();
	return 0;
}