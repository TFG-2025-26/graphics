#include <Windows.h>

#include <rhi/GraphicsConfig.h>
#include <rhi/IGraphicsBackend.h>

#include "GraphicsBackendFactory.h"

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
	const wchar_t CLASS_NAME[] = L"RHIWindowClass";

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
		0,
		CLASS_NAME,
		L"Demo de interfaces",
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT, width, height,

		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (hwnd == 0) return EXIT_SUCCESS;

	ShowWindow(hwnd, nCmdShow);

	GraphicsConfig cfg;
	cfg.api = GraphicsAPI::DX12;
	cfg.windowHandle = hwnd;
	cfg.width = 800;
	cfg.height = 600;
	cfg.vsync = true;
	cfg.shaderBasePath = "assets/shaders/";

	auto backend = CreateGraphicsBackend(cfg);
	if (!backend || !backend->init(cfg)) {
		// log y salir
		fprintf(stderr, "No se ha podido inicializar la biblioteca gráfica...");
		return EXIT_FAILURE;
	}

	MSG msg = {};
	bool running = true;

	while (running) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				running = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!running) break;

		const float clearColor[4] = { 0.6f, 0.7f, 0.8f, 1.0f };
		backend->beginFrame(clearColor);
		backend->drawTestScene();
		backend->endFrame();
	}

	backend->shutdown();
	return EXIT_SUCCESS;
}