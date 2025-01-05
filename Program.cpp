#include <windows.h>
#include <d2d1.h>
#include <string>
#include <wchar.h>
#include <thread>
#include <dwrite.h>
#include <windowsx.h>
#include <commctrl.h>
#include <ShellScalingApi.h>
#include "globals.h"
#include "enums.h"
#include "helperFunctions.h"
#include "BaseWindow.h"
#include "Timer.h"
#include "SettingsWindow.h"
#include "ColorPickerWindow.h"
#include "MainWindow.h"
#include "Program.h"

#pragma comment(lib, "Msimg32.lib")
#pragma comment (lib, "d2d1")

using std::thread; using std::wstring;

// Global variable assignment (defined in MainWindow.h)
settingsStruct appSettings;
HBRUSH hBrushes[25];
HWND hwndMainWindow = nullptr;
MainWindow* pGlobalTimerWindow = NULL; // used for the hook procedure

void AppLoop(MainWindow* win)
{
	while (win->appRunning)
	{
		Sleep(1);
		win->timer1.UpdateTime();
		win->timer2.UpdateTime();
		win->Draw();
	}
}

void KillProgram()
{
	if (pGlobalTimerWindow) pGlobalTimerWindow->appRunning = false;
	PostQuitMessage(0);
}

LRESULT CALLBACK KBHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYUP && pGlobalTimerWindow != NULL && appSettings.startKey != NULL)
	{
		KBDLLHOOKSTRUCT* pKbdHookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		const int startKey = appSettings.startKey;

		if (pKbdHookStruct->vkCode == appSettings.startKey)
		{
			pGlobalTimerWindow->HandleHotKey(KEY_START);
		}
		else if (pKbdHookStruct->vkCode == appSettings.timer1Key)
		{
			pGlobalTimerWindow->HandleHotKey(KEY_TIMER1);
		}
		else if (pKbdHookStruct->vkCode == appSettings.timer2Key)
		{
			pGlobalTimerWindow->HandleHotKey(KEY_TIMER2);
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create the main window
	MainWindow win;
	
	try
	{
		// Create settings file on program first run
		if (!settingsFileExists())
		{
			createSettingsFile();
		}

		// Initiate common controls lib
		InitCommonControls();

		// Initialize brushes
		InitializeBrushes();

		if (!win.Create(L"Timer", 0, 0, 285, 40, WS_EX_TOPMOST | WS_EX_LAYERED, WS_POPUP)) {
			return 0;
		}
		SetLayeredWindowAttributes(win.Window(), 1, 255, LWA_COLORKEY | LWA_ALPHA);

		ShowWindow(win.Window(), nCmdShow);

		// Create variables for settings and color picker windows
		SettingsWindow settings;
		ColorPickerWindow colorPicker;
		settings.pColorPicker = &colorPicker;

		win.pSettingsWindow = &settings;

		// global variables for timer
		pGlobalTimerWindow = &win;
		hwndMainWindow = win.Window();

		// Apply saved settings
		ApplySettings(appSettings);

		// Listen for keys: F1, F2, F While running in the background - Install a hook procedure
		HHOOK kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KBHook, 0, 0);

		// Create a thread for the app loop (ticks)
		thread t1(AppLoop, &win);

		// Handle messages
		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		t1.join();
		return 0;
	}
	catch (const std::exception& e)
	{
		KillProgram();
	}
}