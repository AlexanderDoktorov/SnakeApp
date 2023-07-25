// Змейка.cpp : Определяет точку входа для приложения.

#include "Змейка.h"

#ifdef DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif // DEBUG




int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);

	BaseWindow MyWindow;
	MyWindow.Run();
	
	_CrtDumpMemoryLeaks();
	return 0;
}



