#define _CRTDBG_MAP_ALLOC
#include "Client.h"
#include <stdlib.h>  
#include <crtdbg.h>  
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif


void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	ClipCursor(&desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
}
int main() {

	auto app = new Client();
	int x, y;
	GetDesktopResolution(x, y);

	app->run("AIE", x, y, true);
	delete app;
	_CrtDumpMemoryLeaks();
	return 0;
}