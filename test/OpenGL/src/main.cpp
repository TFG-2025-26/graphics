
#include "IG1App.h"
#include <iostream>

#ifdef WIN32
#include <windows.h>

int
ctrl_handler(int event) // callback
{
	if (event == CTRL_CLOSE_EVENT) { // when the user closes the console
		IG1App::s_ig1app.close();
		return 1;
	} else
		return 0;
}

void
setup_windows_console()
{
	// Handle control events
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), 1);
	// Console output in UTF-8
	SetConsoleOutputCP(CP_UTF8);
	// Cosole output with ANSI escape sequences (color, etc.)
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD cMode = GetConsoleMode(hOut, &cMode);
	SetConsoleMode(hOut, cMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif

int
main(int argc, char* argv[])
{
#ifdef WIN32
	setup_windows_console();
#endif

	try {
		std::cout << "Starting application...\n";
		IG1App::s_ig1app.run();
		std::cout << "Closing application...\n";
	} catch (std::exception& e) {
		std::cout << "\x1b[91;1m[FATAL]\x1b[0m " << e.what() << '\n';
		// std::cin.ignore(INT_MAX, '\n');
	} catch (...) {
		std::cout << "\x1b[91;1m[FATAL]\x1b[0m Closing application...\n";
		// std::cin.ignore(INT_MAX, '\n');
	};

	return 0;
}
