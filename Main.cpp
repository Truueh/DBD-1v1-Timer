#include <windows.h>
#include <d2d1.h>
#include <string>
#include <wchar.h>
#include <thread>
#include <dwrite.h>
#include <windowsx.h>
#include <commctrl.h>
#include "helperFunctions.h"

#pragma comment (lib, "d2d1")

using std::thread; using std::wstring;

// Enum assignment for readable code
enum TimerState
{
	running,
	paused,
	zero
};

settingsStruct appSettings;

HWND hwndMainWindow = nullptr;

class MainWindow;

void ApplySettings(settingsStruct settings) {
	setSettingsStruct(settings);
	appSettings = settings;

	if (hwndMainWindow != NULL) {
		// transparency
		if (appSettings.optionTransparent) { // add transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 0, 255, LWA_COLORKEY | LWA_ALPHA);
		}
		else { // remove transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 1, 255, LWA_COLORKEY | LWA_ALPHA);
		}

		// clickthrough
		if (appSettings.clickthrough) { // make clickthrough
			// Get the current window style
			LONG style = GetWindowLong(hwndMainWindow, GWL_EXSTYLE);

			// Add the new style to the current styles
			style |= WS_EX_TRANSPARENT;

			// Set the new style
			SetWindowLong(hwndMainWindow, GWL_EXSTYLE, style);
		}
	}
}

// A timer struct with all the neccessary functionalities of the timer
class Timer
{
private:
	TimerState timerState;
	int time = 0; // in milliseconds
	SYSTEMTIME startTime;
	SYSTEMTIME updatingTime;
	
	int SubtractTimes(SYSTEMTIME t1, SYSTEMTIME t2)
	{
		int seconds = 0;
		int minutes = 0;
		int millis = 0;

		int t1Millis = (t1.wMinute * 60 * 1000) + (t1.wSecond * 1000) + t1.wMilliseconds;
		int t2Millis = (t2.wMinute * 60 * 1000) + (t2.wSecond * 1000) + t2.wMilliseconds;

		if (t1Millis > t2Millis) {
			t2Millis += (60 * 60 * 1000);
		}

		millis = t2Millis - t1Millis;

		return millis;
	}

	wstring GetTimeAsText()
	{
		int minutesInt = 0;
		int secondsInt = 0;
		int millisInt = time;

		secondsInt = millisInt / 1000;
		minutesInt = secondsInt / 60;
		millisInt = millisInt % 1000;
		secondsInt = secondsInt % 60;

		wstring secondsStr = std::to_wstring(secondsInt);
		wstring minutesStr = std::to_wstring(minutesInt);
		wstring millisStr = std::to_wstring(millisInt);

		if (millisStr.size() == 1) {
			millisStr += '0';
		}

		wstring text = L"              ";

		if (minutesInt < 1) {
			if (secondsInt < 10) {
				text[0] = secondsStr[0];
				text[1] = '.';
				text[2] = millisStr[0];
				text[3] = millisStr[1];
			}
			else {
				text[0] = secondsStr[0];
				text[1] = secondsStr[1];
				text[2] = '.';
				text[3] = millisStr[0];
				text[4] = millisStr[1];
			}
		}
		else {
			if (minutesInt < 10) {
				text[0] = minutesStr[0];
				text[1] = ':';

				if (secondsInt < 10) {
					text[2] = '0'; text[3] = secondsStr[0];
				}
				else {
					text[2] = secondsStr[0]; text[3] = secondsStr[1];
				}
				text[4] = '.';
				text[5] = millisStr[0];
			}
			else {
				text[0] = minutesStr[0];
				text[1] = minutesStr[1];
				text[2] = ':';

				if (secondsInt < 10) {
					text[3] = '0'; text[4] = secondsStr[0];
				}
				else {
					text[3] = secondsStr[0]; text[4] = secondsStr[1];
				}
				text[5] = '.';
				text[6] = millisStr[0];
			}
		}

		return text;
	}

public:
	Timer() {
		time = 0;
		timerState = TimerState::zero;
	}

	TimerState GetTimerState()
	{
		return timerState;
	}

	int GetTimeInMillis()
	{
		return time;
	}

	void StartTimer()
	{
		timerState = TimerState::running;
		GetSystemTime(&startTime);
	}

	void StopTimer()
	{
		timerState = TimerState::paused;
	}

	void ResetTimer()
	{
		timerState = TimerState::zero;
		GetLocalTime(&startTime);
		GetLocalTime(&updatingTime);
		time = SubtractTimes(startTime, updatingTime);
	}

	void UpdateTime()
	{
		if (timerState == TimerState::running)
		{
			GetSystemTime(&updatingTime);
			time = SubtractTimes(startTime, updatingTime);
		}
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, IDWriteTextFormat* pTextFormat, D2D1_RECT_F rectF, ID2D1SolidColorBrush* pBrush) // Only call from within an active render target begin draw scope
	{
		if (pRenderTarget != NULL)
		{
			wstring timeStr = GetTimeAsText();

			const WCHAR* timeText = timeStr.c_str();

			pRenderTarget->DrawTextW(
				timeText,
				8,
				pTextFormat,
				rectF,
				pBrush);
		}
	}
};

// Template class for specific window's classes to inherit from
template <class DERIVED_CLASS> class BaseWindow
{
public:
	BaseWindow() : m_hwnd(NULL) { };

	HWND Window() const { return m_hwnd; }

	bool appRunning = false;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_CLASS* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* cStruct = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_CLASS*)cStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_CLASS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis != NULL)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	BOOL Create(
		LPCWSTR lpWindowName,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int width = CW_USEDEFAULT,
		int height = CW_USEDEFAULT,
		DWORD dwExStyle = WS_OVERLAPPEDWINDOW,
		DWORD dwStyle = 0,
		HWND parent = NULL,
		HMENU hMenu = NULL,
		HINSTANCE hInstance = NULL,
		LPVOID lpParam = 0
	) {
		// create a window class
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = DERIVED_CLASS::WindowProc;
		wc.lpszClassName = ClassName();
		wc.hInstance = NULL;
		wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y, width, height, parent, hMenu, hInstance, this);

		winSize[0] = width; winSize[1] = height;

		return (m_hwnd ? TRUE : FALSE);
	}

protected:
	HWND m_hwnd;
	int winSize[2] = { 0, 0 };

	virtual LPCWSTR ClassName() const { return 0; }
	virtual LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam) = 0;
};

// Release pointers safely
template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// The class responsible for the settings window
class SettingsWindow : public BaseWindow<SettingsWindow>
{
private:
	int windowWidth = 310;
	int windowHeight = 410;
	settingsStruct tempSettings;

	void InitializeWindow()
	{
		// preset values
		int xTitle = 15;
		int yOffset = 35;

		int xHotkey = windowWidth / 2 + 15;
		int widthHotkey = 150;
		int heightHotkey = 20;
		int sizeCheckbox = 40; // mostly irrelevant since there is no text

		// Initialize headers
		HWND hwndTitleHotkeys = CreateWindowEx(0, WC_STATIC, L"Hotkeys", WS_VISIBLE | WS_CHILDWINDOW, 10, 5, 60, 40, m_hwnd, 0, NULL, NULL);
		HWND hwndTextStart = CreateWindowEx(0, WC_STATIC, L"Start / Stop / Reset", WS_VISIBLE | WS_CHILDWINDOW, xTitle, yOffset * 1, 150, 40, m_hwnd, 0, NULL, NULL);
		HWND hwndTextTimer1 = CreateWindowEx(0, WC_STATIC, L"Timer 1", WS_VISIBLE | WS_CHILDWINDOW, xTitle, yOffset * 2, 150, 40, m_hwnd, 0, NULL, NULL);
		HWND hwndTextTimer2 = CreateWindowEx(0, WC_STATIC, L"Timer 2", WS_VISIBLE | WS_CHILDWINDOW, xTitle, yOffset * 3, 150, 40, m_hwnd, 0, NULL, NULL);
		HWND hwndTextTransparentBackground = CreateWindowEx(0, WC_STATIC, L"Transparent", WS_VISIBLE | WS_CHILDWINDOW, xTitle, yOffset * 4, 150, 40, m_hwnd, 0, NULL, NULL);
		HWND hwndTextCheckboxClickthrough = CreateWindowEx(0, WC_STATIC, L"Clickthrough", WS_VISIBLE | WS_CHILDWINDOW, xTitle, yOffset * 5, 150, 40, m_hwnd, 0, NULL, NULL);

		// Initialize hotkey detectors
		HWND hwndHotkeyStart = CreateWindowEx(0, HOTKEY_CLASS, L"", WS_VISIBLE | WS_CHILDWINDOW, xHotkey, yOffset * 1, widthHotkey, heightHotkey, m_hwnd, (HMENU)CID_START, NULL, NULL);
		HWND hwndHotkeyTimer1 = CreateWindowEx(0, HOTKEY_CLASS, L"", WS_VISIBLE | WS_CHILDWINDOW, xHotkey, yOffset * 2, widthHotkey, heightHotkey, m_hwnd, (HMENU)CID_TIMER1, NULL, NULL);
		HWND hwndHotkeyTimer2 = CreateWindowEx(0, HOTKEY_CLASS, L"", WS_VISIBLE | WS_CHILDWINDOW, xHotkey, yOffset * 3, widthHotkey, heightHotkey, m_hwnd, (HMENU)CID_TIMER2, NULL, NULL);
		HWND hwndCheckboxTransparentBackground = CreateWindowEx(0, WC_BUTTON, L"",BS_CHECKBOX | WS_VISIBLE | WS_CHILDWINDOW | BS_AUTOCHECKBOX, xHotkey, yOffset * 4 - 10, sizeCheckbox, sizeCheckbox, m_hwnd, (HMENU)CID_TRANSPARENT_CB, NULL, NULL);
		HWND hwndCheckboxClickthrough = CreateWindowEx(0, WC_BUTTON, L"", BS_CHECKBOX | WS_VISIBLE | WS_CHILDWINDOW | BS_AUTOCHECKBOX, xHotkey, yOffset * 5 - 10, sizeCheckbox, sizeCheckbox, m_hwnd, (HMENU)CID_CLICKTHROUGH, NULL, NULL);

		// Initialize exit controls
		HWND hwndOKButton = CreateWindowEx(0, WC_BUTTON, L"OK", WS_VISIBLE | WS_CHILDWINDOW, windowWidth - 120, windowHeight - 30, 50, 25, m_hwnd, (HMENU)CID_OK, NULL, NULL);
		HWND hwndCancelButton = CreateWindowEx(0, WC_BUTTON, L"CANCEL", WS_VISIBLE | WS_CHILDWINDOW, windowWidth - 60, windowHeight - 30, 70, 25, m_hwnd, (HMENU)CID_CANCEL, NULL, NULL);

		tempSettings = getSettingsStruct();

		// Apply current settings
		SendMessage(hwndHotkeyStart, HKM_SETHOTKEY, appSettings.startKey, 0);
		SendMessage(hwndHotkeyTimer1, HKM_SETHOTKEY, appSettings.timer1Key, 0);
		SendMessage(hwndHotkeyTimer2, HKM_SETHOTKEY, appSettings.timer2Key, 0);
		SendMessage(hwndCheckboxTransparentBackground, BM_SETCHECK, appSettings.optionTransparent, 0);
		SendMessage(hwndCheckboxClickthrough, BM_SETCHECK, appSettings.clickthrough, 0);
	}

	void HandleControlCommand(LPARAM lParam)
	{
		HWND hwndCtrl = reinterpret_cast<HWND>(lParam); // clicked item handle
		int controlID = GetDlgCtrlID(hwndCtrl); // retrieve control ID
		WORD virtualKey = (WORD)SendMessage(hwndCtrl, HKM_GETHOTKEY, 0, 0); // Retrieve HOTKEY pressed

		switch (controlID)
		{
		case CID_OK: // ok
			ApplySettings(tempSettings);
			DestroyWindow(m_hwnd);
			break;
		case CID_CANCEL: // cancel
			DestroyWindow(m_hwnd);
			break;
		case CID_START: // Start
			tempSettings.startKey = virtualKey;
			break;
		case CID_TIMER1: // Timer 1
			tempSettings.timer1Key = virtualKey;
			break;
		case CID_TIMER2: // Timer 2
			tempSettings.timer2Key = virtualKey;
			break;
		case CID_TRANSPARENT_CB: // Transparent background checkbox
			tempSettings.optionTransparent = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
			break;
		case CID_CLICKTHROUGH:
			tempSettings.clickthrough = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
			break;
		}
	}

public:
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (wMsg)
		{
		case WM_CREATE:
		{
			InitializeWindow();
			return 0;
		}
		case WM_DESTROY:
			m_hwnd = NULL;
			return 0;
		case WM_COMMAND: // Control item clicked
			HandleControlCommand(lParam);
			return 0;
		}
		return DefWindowProc(Window(), wMsg, wParam, lParam);
	}

	LPCWSTR ClassName() const { return L"Settings Window"; }
};

// The class responsible for the main window of the app
class MainWindow : public BaseWindow<MainWindow>
{
	// Resources
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;
	ID2D1SolidColorBrush* pBrushGreen;
	ID2D1SolidColorBrush* pBrushBlue;
	ID2D1SolidColorBrush* pBrushRed;

	// Writing Resources
	IDWriteFactory* pWriteFactory;
	IDWriteTextFormat* pTextFormat;

private:
	Timer* activeTimer;

	BOOL mouseDown = false;
	int clickMousePos[2] = {0, 0};

	HRESULT CreateGraphicsResources()
	{
		HRESULT hr = S_OK;
		
		// Set up render target and brush
		if (pRenderTarget == NULL)
		{
			RECT rc;
			GetClientRect(m_hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

			D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
				96.0f, 96.0f,
				D2D1_RENDER_TARGET_USAGE_NONE,
				D2D1_FEATURE_LEVEL_DEFAULT
			);

			hr = pFactory->CreateHwndRenderTarget(
				rtProperties,
				D2D1::HwndRenderTargetProperties(m_hwnd, size),
				&pRenderTarget
			);

			if (SUCCEEDED(hr)) {
				const D2D1_COLOR_F greenColor = D2D1::ColorF(0.0f, 1, 0.4f);
				hr = pRenderTarget->CreateSolidColorBrush(greenColor, &pBrushGreen);

				const D2D1_COLOR_F blueColor = D2D1::ColorF(0.0f, 0.6f, 1);
				hr = pRenderTarget->CreateSolidColorBrush(blueColor, &pBrushBlue);

				const D2D1_COLOR_F redColor = D2D1::ColorF(1, 0.3f, 0.3f);
				hr = pRenderTarget->CreateSolidColorBrush(redColor, &pBrushRed);
			}
		}

		return hr;
	}

	HRESULT CreateDeviceIndependentResources()
	{
		HRESULT hr = S_OK;
		// Set up write factory
		if (pWriteFactory == NULL)
		{
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory)
				, reinterpret_cast<IUnknown**>(&pWriteFactory));

			// Set up text format
			static const WCHAR fontName[] = L"Verdana";
			static const FLOAT fontSize = 34;

			if (SUCCEEDED(hr))
			{
				pWriteFactory->CreateTextFormat(
					fontName,
					NULL,
					DWRITE_FONT_WEIGHT_EXTRA_LIGHT,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_EXTRA_EXPANDED,
					fontSize,
					L"",
					&pTextFormat
				);

				if (SUCCEEDED(hr))
				{
					// Center the text horizontally and vertically.
					pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

					pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				}
			}
		}

		return hr;
	}

	void DiscardGraphicsResources()
	{
		SafeRelease(&pRenderTarget);
		SafeRelease(&pBrushGreen);
		SafeRelease(&pBrushBlue);
		SafeRelease(&pBrushRed);
		SafeRelease(&pWriteFactory);
		SafeRelease(&pTextFormat);
	}

	void HandlePainting()
	{
		HRESULT hrGraphics = CreateGraphicsResources();
		HRESULT hrWrite = CreateDeviceIndependentResources();

		if (SUCCEEDED(S_OK))
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hwnd, &ps);
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));

			D2D1_RECT_F rect1 = D2D1::RectF(0, 0, winSize[0] / 2, winSize[1]);
			D2D1_RECT_F rect2 = D2D1::RectF(winSize[0] / 2, 0, winSize[0], winSize[1]);

			if (SUCCEEDED(hrWrite))
			{
				if (activeTimer != NULL)
				{
					// Select color for timer 2
					ID2D1SolidColorBrush* pBrushTimer2;
					if (timer1.GetTimeInMillis() > 0
						&& timer1.GetTimeInMillis() - timer2.GetTimeInMillis() <= 20000
						&& (timer2.GetTimerState() == TimerState::running || timer2.GetTimerState() == TimerState::paused)
						&& timer1.GetTimeInMillis() - timer2.GetTimeInMillis() > 0)
					{
						pBrushTimer2 = pBrushRed;
					}
					else if (activeTimer == &timer2) {
						pBrushTimer2 = pBrushBlue;
					}
					else {
						pBrushTimer2 = pBrushGreen;
					}

					// Draw timers
					if (activeTimer == &timer1) {
						timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushBlue);
					}
					else {
						timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushGreen);
					}
					timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushTimer2);
				}
				else
				{
					timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushGreen);
					timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushGreen);
				}
			}

			hrGraphics = pRenderTarget->EndDraw();

			if (FAILED(hrGraphics) || hrGraphics == D2DERR_RECREATE_TARGET)
			{
				DiscardGraphicsResources();
			}
			EndPaint(m_hwnd, &ps);
		}
	}

	void HandleWindowDrag(LPARAM lParam)
	{
		RECT windowPos;
		GetWindowRect(m_hwnd, &windowPos);
		int currPos[2] = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		int xToMove = windowPos.left + (currPos[0] - clickMousePos[0]);
		int yToMove = windowPos.top + (currPos[1] - clickMousePos[1]);

		SetWindowPos(m_hwnd, NULL, xToMove, yToMove, winSize[0], winSize[1], 0);
	}

public:
	Timer timer1 = Timer();
	Timer timer2 = Timer();

	SettingsWindow* pSettingsWindow;

	LPCWSTR ClassName() const { return L"Main Window"; }
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (wMsg)
		{
		case WM_CREATE:
			if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory))) {
				return -1;
			}
			appSettings = getSettingsStruct();
			appRunning = true;
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			appRunning = false;
			return 0;
		case WM_LBUTTONDOWN:
			mouseDown = true;
			clickMousePos[0] = GET_X_LPARAM(lParam);
			clickMousePos[1] = GET_Y_LPARAM(lParam);
			return 0;
		case WM_LBUTTONUP:
			mouseDown = false;
			return 0;
		case WM_MOUSEMOVE:
			if (mouseDown)
			{
				HandleWindowDrag(lParam);
			}
			return 0;
		case WM_COMMAND:
			switch (wParam)
			{
			case MENU_SETTINGS:
				if (pSettingsWindow->Window() == NULL) // dont create multiple settings windows
				{
					// Create and show settings window
					if (!pSettingsWindow->Create(L"Settings", 500, 200, 350, 450, 0, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, m_hwnd, 0, 0, NULL)) {
						return 0;
					}

					ShowWindow(pSettingsWindow->Window(), SW_SHOW);
				}
				else
				{
					SetForegroundWindow(pSettingsWindow->Window());
				}
				return 0;
			case MENU_QUIT:
				PostQuitMessage(0);
				appRunning = false;
				return 0;
			}
			return 0;
		case WM_CONTEXTMENU:
			{
				// mouse pos
				int mouseX = GET_X_LPARAM(lParam);
				int mouseY = GET_Y_LPARAM(lParam);

				// create popup menu
				HMENU hMenu = CreatePopupMenu();
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, MENU_QUIT, L"Quit");
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 100, L"");
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, MENU_SETTINGS, L"Settings");
				SetForegroundWindow(m_hwnd);
				TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, mouseX, mouseY, 0, m_hwnd, NULL);
				return 0;
			}
		}
		return DefWindowProc(Window(), wMsg, wParam, lParam);
	}
	void HandleHotKey(int code)
	{
		switch (code)
		{
		case KEY_TIMER1: // timer1 key
			activeTimer = &timer1;
			break;
		case KEY_TIMER2: // timer2 key
			activeTimer = &timer2;
			break;
		case KEY_START: // start key
			if (activeTimer != NULL)
			{
				if (activeTimer->GetTimerState() == TimerState::zero) {
					activeTimer->StartTimer();
				}
				else if (activeTimer->GetTimerState() == TimerState::running) {
					activeTimer->StopTimer();
				}
				else {
					activeTimer->ResetTimer();
				}
			}
			break;
		}
	}

	void Draw() {
		HandlePainting();
	}
};

// Global reference to the main window class instance (for the hook proc)
MainWindow* pGlobalTimerWindow = NULL;

// App Loop logic
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

// The hook procedure to listen for kestrokes f1, f2 and f
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

// The main function
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create settings file on program first run
	if (!settingsFileExists())
	{
		createSettingsFile();
	}

	// Initiate common controls lib
	InitCommonControls();

	// Create the main window
	MainWindow win;

	if (!win.Create(L"Timer", 0, 0, 285, 40, WS_EX_TOPMOST | WS_EX_LAYERED, WS_POPUP)) {
		return 0;
	}
	SetLayeredWindowAttributes(win.Window(), 1, 255, LWA_COLORKEY | LWA_ALPHA);

	ShowWindow(win.Window(), nCmdShow);

	// Create variables for settings window
	SettingsWindow settings;
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