#include "MainWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd) {
	(void) hPrevInstance;
	(void) lpCmdLine;

	// Forward to MainWindow class
	return MainWindow(hInstance, nShowCmd).run();

}
