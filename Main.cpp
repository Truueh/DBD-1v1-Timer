#include <windows.h>
#include <d2d1.h>
#include <string>
#include <wchar.h>
#include <thread>
#include <dwrite.h>
#include <windowsx.h>
#include <commctrl.h>
#include <ShellScalingApi.h>
#include "helperFunctions.h"
#include <Xinput.h>
#include <dinput.h>

#pragma comment (lib, "xinput.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment (lib, "d2d1")

using std::thread; using std::wstring;

// Enum assignment for readable code
enum TimerState
{
	running,
	paused,
	zero
};

enum MousePos
{
	topLeft,
	top,
	topRight,
	right,
	bottomRight,
	bottom,
	bottomLeft,
	left,
	none
};

// static variable assignment
settingsStruct appSettings;

HWND hwndMainWindow = nullptr;

HBRUSH hBrushes[25];

class MainWindow;

// static methods

// load a bitmap resource 
HBITMAP LoadBitmapResource(int bitmap) {
	HBITMAP hBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmap));
	if (!hBitmap) {
		MessageBox(NULL, L"Failed to load bitmap!", L"Error", MB_OK);
	}

	return hBitmap;
}

// Initialize a list of brusehs for each selectable color
void InitializeBrushes()
{
	COLORREF colors[25] = {
		RGB(255, 0, 0), RGB(255, 77, 0), RGB(255, 116, 0), RGB(255, 154, 0), RGB(255, 193, 0),
		RGB(1, 55, 125), RGB(0, 157, 209), RGB(151, 231, 245), RGB(115, 211, 72), RGB(38, 177, 112),
		RGB(49, 0, 74), RGB(51, 0, 123), RGB(76, 0, 164), RGB(131, 0, 196), RGB(171, 0, 255),
		RGB(255, 0, 255), RGB(192, 64, 255), RGB(128, 128, 255), RGB(64, 182, 255), RGB(0, 255, 255),
		RGB(1, 1, 1), RGB(35, 35, 35), RGB(85, 85, 85), RGB(182, 176, 169), RGB(237, 231, 224)
	};

	for (size_t i = 0; i < 25; i++)
	{
		hBrushes[i] = CreateSolidBrush(colors[i]);
	}
}

// A procedure for windows of type "control", used to assign a font
BOOL CALLBACK ControlProc(HWND hControl, LPARAM lParam)
{
	HFONT hFont = (HFONT)lParam;
	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);

	return true;
}

// Set a specific control's font (title style)
void SetTitleFont(HWND hControl)
{
	HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// Apply the default font to a specific window's child controls
void SetControlsFont(HWND hWnd)
{
	HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	EnumChildWindows(hWnd, ControlProc, (LPARAM)hFont);
}

void ApplySettings(settingsStruct settings) {
	setSettingsStruct(settings); // write to json file (settings.json)
	appSettings = settings; // save static settings variable

	if (hwndMainWindow != NULL) {
		// transparency
		if (appSettings.optionTransparent) { // add transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 0, 0, LWA_COLORKEY);
		}
		else { // remove transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 0, 255, LWA_ALPHA);
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

// A timer class with all the neccessary functionalities of the timer
class Timer
{
private:
	TimerState timerState;
	int time = 0; // in milliseconds
	SYSTEMTIME startTime;
	SYSTEMTIME updatingTime;
	
	// Get milliseconds difference between system times
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

public:
	Timer() {
		time = 0;
		timerState = TimerState::zero;
	}

	// Getters and public methods
	TimerState GetTimerState()
	{
		return timerState;
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
	// public variables / methods
	BaseWindow() : m_hwnd(NULL) { };

	HWND Window() const { return m_hwnd; }

	bool appRunning = false; // saves if the main window is running and signal to break the program loop if it's not (exit app)

	// The actual window procedure every message is sent to. responsible for linking windows to class instances and forward messages to their handlers.
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_CLASS* pThis = NULL;

		if (uMsg == WM_NCCREATE) // if a window has just been created, link it to this class instance
		{
			CREATESTRUCT* cStruct = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_CLASS*)cStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else // if it's just another message to an existing window, save it's class instance to this local variable
		{
			pThis = (DERIVED_CLASS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis != NULL) // if the class instance exsists (it should) forward the message to the relevant handler method
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	// Create an HWND (with the windowproc of basewindow that will link this class instance to the HWND)
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

	// protected methods to be rewritten by derived classes
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

// Class for the color picker window
class ColorPickerWindow : public BaseWindow<ColorPickerWindow>
{
private:
	// Variables
	HWND hColorButtons[25];
	HBRUSH hPreviewColor;
	HWND hPreviewColorButton;
	int previewColorIndex;

	// Methods
	// Create and store the squared controls that represent selectable colors
	void InitializeColorButtons()
	{
		int i = 0;
		int baseRowPos = 40;
		int baseColPos = 20;
		int buttonSize = 30;
		int offset = 35;

		for (size_t row = 0; row < 5; row++)
		{
			for (size_t col = 0; col < 5; col++)
			{
				hColorButtons[i] = CreateWindowEx(0, WC_BUTTON, L"", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, baseRowPos + offset * col, baseColPos + offset * row, buttonSize, buttonSize, m_hwnd, (HMENU)i, NULL, NULL);

				i++;
			}
		}

		// Preview Color
		hPreviewColorButton = CreateWindowEx(0, WC_BUTTON, L"", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 105, 210, 40, 40, m_hwnd, (HMENU)COLOR_PREVIEW, NULL, NULL);
		switch (controlID)
		{
		case COLOR_CTR_TIMER:
			hPreviewColor = hBrushes[pTempSettings->colors.timerColor];
			previewColorIndex = pTempSettings->colors.timerColor;
			break;
		case COLOR_CTR_SELECTED_TIMER:
			hPreviewColor = hBrushes[pTempSettings->colors.selectedTimerColor];
			previewColorIndex = pTempSettings->colors.selectedTimerColor;
			break;
		case COLOR_CTR_LAST_SECONDS:
			hPreviewColor = hBrushes[pTempSettings->colors.lastSecondsColor];
			previewColorIndex = pTempSettings->colors.lastSecondsColor;
			break;
		case COLOR_CTR_BACKGROUND:
			hPreviewColor = hBrushes[pTempSettings->colors.backgroundColor];
			previewColorIndex = pTempSettings->colors.backgroundColor;
			break;
		}
	}

	// Method to be called when the window is created
	void InitializeWindow()
	{
		InitializeColorButtons();

		// Initialize exit controls
		HWND hwndOKButton = CreateWindowEx(0, WC_BUTTON, L"OK", WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 150, SIZE_COLORPICKER_HEIGHT - 70, 50, 25, m_hwnd, (HMENU)CID_OK, NULL, NULL);
		HWND hwndCancelButton = CreateWindowEx(0, WC_BUTTON, L"CANCEL", WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 90, SIZE_COLORPICKER_HEIGHT - 70, 70, 25, m_hwnd, (HMENU)CID_CANCEL, NULL, NULL);
	
		SetControlsFont(m_hwnd);
	}

	// Apply changes to the temporary struct the settings window is using
	void UpdateSettings()
	{
		switch (controlID)
		{
		case COLOR_CTR_TIMER:
			pTempSettings->colors.timerColor = previewColorIndex;
			break;
		case COLOR_CTR_SELECTED_TIMER:
			pTempSettings->colors.selectedTimerColor = previewColorIndex;
			break;
		case COLOR_CTR_LAST_SECONDS:
			pTempSettings->colors.lastSecondsColor = previewColorIndex;
			break;
		case COLOR_CTR_BACKGROUND:
			pTempSettings->colors.backgroundColor = previewColorIndex;
			break;
		}

		RedrawWindow(GetWindow(m_hwnd, GW_OWNER), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	// Called when a control window is clicked
	void HandleControlCommand(LPARAM lParam)
	{
		// retrieve clicked control information
		HWND hwndCtrl = reinterpret_cast<HWND>(lParam); // clicked item handle
		int CID = GetDlgCtrlID(hwndCtrl); // retrieve control ID

		// clicked a color
		if (CID >= 0 && CID < 25)
		{
			hPreviewColor = hBrushes[CID];
			previewColorIndex = CID;
			RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			return;
		}

		// clicked some other control
		switch (CID)
		{
		case CID_OK:
			UpdateSettings();
			DestroyWindow(m_hwnd);
			break;
		case CID_CANCEL:
			DestroyWindow(m_hwnd);
			break;
		}
	}

	public:
	// public variables
	int controlID; // ID of the control that summoned the color picker window
	settingsStruct* pTempSettings; // Reference to the settings window's settings struct

	// protected methods rewrite (from BaseClass)
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
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
			if (pDIS->CtlID == COLOR_PREVIEW)
			{
				FillRect(pDIS->hDC, &pDIS->rcItem, hPreviewColor);
			}
			else
			{
				FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[pDIS->CtlID]);
			}

			return TRUE;
		}
		}
		return DefWindowProc(Window(), wMsg, wParam, lParam);
	}

	LPCWSTR ClassName() const { return L"Settings Window"; }
};

// The class responsible for the settings window
class SettingsWindow : public BaseWindow<SettingsWindow>
{
private:
	settingsStruct tempSettings;
	HBITMAP mouseBitmap;
	HBITMAP controllerBitmap;

	// Window setup
	void InitializeWindow()
	{
		// retrieve settings
		tempSettings = getSafeSettingsStruct();

		// Set up text
		InitializeTextControls();

		// Set up buttons and hotkeys
		InitializeButtonControls();

		// Set up bitmaps
		InitializeBitmaps();
	}

	// Initialize bitmaps
	void InitializeBitmaps()
	{
		mouseBitmap = LoadBitmapResource(IDB_MOUSE);
		controllerBitmap = LoadBitmapResource(IDB_CONTROLLER);
	}

	// Display bitmaps
	void DisplayBitmaps()
	{
		// preset values
		int mouseX = SIZE_SETTINGS_WIDTH / 2 + 50; int mouseY = 35;
		int controllerX = mouseX + 70; int controllerY = 35;
		int bitmapSize = 25;
		int originalBitmapSize = 400;
		COLORREF ignoreColor = RGB(100, 100, 100);

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hwnd, &ps);

		// Create a compatible device context
		HDC hMemDC = CreateCompatibleDC(hdc);

		// Select the mouse bitmap into the memory DC
		HGDIOBJ oldBitmap = SelectObject(hMemDC, mouseBitmap);

		// Scale bitmaps
		TransparentBlt(hdc,
			mouseX, mouseY,			         // Target x, y coordinates
			bitmapSize,				         // Target width
			bitmapSize,						 // Target height
			hMemDC,
			0, 0,			                 // Source x, y coordinates
			originalBitmapSize,				 // Source width
			originalBitmapSize,				 // Source height
			ignoreColor);

		// Select the controller bitmap into the memory DC
		oldBitmap = SelectObject(hMemDC, controllerBitmap);

		// Scale bitmaps
		TransparentBlt(hdc,
			controllerX, controllerY,			         // Target x, y coordinates
			bitmapSize,				         // Target width
			bitmapSize,						 // Target height
			hMemDC,
			0, 0,			                 // Source x, y coordinates
			originalBitmapSize,				 // Source width
			originalBitmapSize,				 // Source height
			ignoreColor);

		// Cleanup
		SelectObject(hMemDC, oldBitmap);
		DeleteDC(hMemDC);

		EndPaint(m_hwnd, &ps);
	}

	// Initialize headers
	void InitializeTextControls()
	{
		// preset values
		int xTitle = 15;
		int yOffset = 35;

		// Initialize headers
		HWND hwndTitleHotkeys = CreateControl(WC_STATIC, L"Hotkeys", SIZE_SETTINGS_WIDTH / 2 - 40, 5, 60, 40);
		HWND hwndTitleColors = CreateControl(WC_STATIC, L"Colors", SIZE_SETTINGS_WIDTH / 2 - 40, yOffset * 8, 60, 40);

		// Hotkey titles
		HWND hwndTextStart = CreateControl(WC_STATIC, L"Start / Stop / Reset", xTitle, yOffset * 2, 150, 40);
		HWND hwndTextTimer1 = CreateControl(WC_STATIC, L"Timer 1", xTitle, yOffset * 3, 150, 40);
		HWND hwndTextTimer2 = CreateControl(WC_STATIC, L"Timer 2", xTitle, yOffset * 4, 150, 40);
		HWND hwndTextTransparentBackground = CreateControl(WC_STATIC, L"Transparent Background", xTitle, yOffset * 5, 150, 40);
		HWND hwndTextCheckboxClickthrough = CreateControl(WC_STATIC, L"Clickthrough (resets when app is closed)", xTitle, yOffset * 6, 150, 40);

		// Break line
		HWND hwndBreakLine = CreateControl(WC_STATIC, NULL, 15, yOffset * 8 - 20, 300, 5, NULL, SS_ETCHEDHORZ);

		// Color options names
		HWND hwndTextColorTimer = CreateControl(WC_STATIC, L"Timer", xTitle, yOffset * 9, 150, 40);
		HWND hwndTextColorSelectedTimer = CreateControl(WC_STATIC, L"Selected Timer", xTitle, yOffset * 10, 150, 40);
		HWND hwndTextColorWinCon = CreateControl(WC_STATIC, L"Last 20 Seconds", xTitle, yOffset * 11, 150, 40);
		HWND hwndTextColorBackground = CreateControl(WC_STATIC, L"Background", xTitle, yOffset * 12, 150, 40);

		// Copyright text
		HWND hwndCopyright = CreateControl(WC_STATIC, L"© Truueh 2024", 10, SIZE_SETTINGS_HEIGHT - 65, 100, 40);

		// Apply fonts
		SetControlsFont(m_hwnd);
		SetTitleFont(hwndTitleColors);
		SetTitleFont(hwndTitleHotkeys);
		SetCopyrightFont(hwndCopyright);
	}

	void InitializeButtonControls()
	{
		// preset values
		int yOffset = 35;

		int xHotkey = SIZE_SETTINGS_WIDTH / 2 + 50;
		int xHotkeyCon = xHotkey + 70;
		int xColorButton = xHotkeyCon - 80;
		int widthHotkey = 25;
		int heightHotkey = 25;
		int sizeCheckbox = 40;

		// Hotkey buttons - Mouse
		HWND hHotkeyStart = CreateControl(HOTKEY_CLASS, L"", xHotkey, yOffset * 2, widthHotkey, heightHotkey, CID_START);
		HWND hHotkeyTimer1 = CreateControl(HOTKEY_CLASS, L"", xHotkey, yOffset * 3, widthHotkey, heightHotkey, CID_TIMER1);
		HWND hHotkeyTimer2 = CreateControl(HOTKEY_CLASS, L"", xHotkey, yOffset * 4, widthHotkey, heightHotkey, CID_TIMER2);
		HWND hCbTransparentBg = CreateControl(WC_BUTTON, L"", xHotkeyCon, yOffset * 5 - 10, sizeCheckbox, sizeCheckbox, CID_TRANSPARENT_CB, BS_CHECKBOX | BS_AUTOCHECKBOX);
		HWND hCbClickthrough = CreateControl(WC_BUTTON, L"", xHotkeyCon, yOffset * 6 - 10, sizeCheckbox, sizeCheckbox, CID_CLICKTHROUGH, BS_CHECKBOX | BS_AUTOCHECKBOX);

		// Hotkey buttons - Controller
		HWND hHotkeyStartCon = CreateControl(HOTKEY_CLASS, L"", xHotkeyCon, yOffset * 2, widthHotkey, heightHotkey, -1);
		HWND hHotkeyTimer1Con = CreateControl(HOTKEY_CLASS, L"", xHotkeyCon, yOffset * 3, widthHotkey, heightHotkey, -1);
		HWND hHotkeyTimer2Con = CreateControl(HOTKEY_CLASS, L"", xHotkeyCon, yOffset * 4, widthHotkey, heightHotkey, -1);
		
		// Color buttons
		HWND hwndButtonColorTimer = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 9, 90, 15, COLOR_CTR_TIMER, BS_OWNERDRAW);
		HWND hwndButtonColorSelectedTimer = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 10, 90, 15, COLOR_CTR_SELECTED_TIMER, BS_OWNERDRAW);
		HWND hwndButtonColorLastSeconds = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 11, 90, 15, COLOR_CTR_LAST_SECONDS, BS_OWNERDRAW);
		HWND hwndButtonColorBackground = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 12, 90, 15, COLOR_CTR_BACKGROUND, BS_OWNERDRAW);

		// Initialize exit controls
		HWND hwndOKButton = CreateControl(WC_BUTTON, L"OK", SIZE_SETTINGS_WIDTH - 160, SIZE_SETTINGS_HEIGHT - 80, 50, 25, CID_OK);
		HWND hwndCancelButton = CreateControl(WC_BUTTON, L"CANCEL", SIZE_SETTINGS_WIDTH - 100, SIZE_SETTINGS_HEIGHT - 80, 70, 25, CID_CANCEL);

		// Apply current settings
		SendMessage(hHotkeyStart, HKM_SETHOTKEY, appSettings.startKey, 0);
		SendMessage(hHotkeyTimer1, HKM_SETHOTKEY, appSettings.timer1Key, 0);
		SendMessage(hHotkeyTimer2, HKM_SETHOTKEY, appSettings.timer2Key, 0);
		SendMessage(hCbTransparentBg, BM_SETCHECK, appSettings.optionTransparent, 0);
		SendMessage(hCbClickthrough, BM_SETCHECK, appSettings.clickthrough, 0);
	}

	// Function for creating a control
	HWND CreateControl(LPCWSTR className, LPCWSTR controlName, int x, int y, int width, int height, int id=NULL, long ADDITIONAL_STYLE=0) {
		return CreateWindowEx(0, className, controlName, WS_VISIBLE | WS_CHILDWINDOW | ADDITIONAL_STYLE, x, y, width, height, m_hwnd, (HMENU)id, NULL, NULL);
	}

	// Apply the font for the copyright style to a specific control
	void SetCopyrightFont(HWND hControl)
	{
		HFONT hFont = CreateFont(
			14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
		);

		SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
	}

	// Method to handle controls clicked
	void HandleControlCommand(LPARAM lParam)
	{
		HWND hwndCtrl = reinterpret_cast<HWND>(lParam); // clicked item handle
		int controlID = GetDlgCtrlID(hwndCtrl); // retrieve control ID
		WORD virtualKey = (WORD)SendMessage(hwndCtrl, HKM_GETHOTKEY, 0, 0); // Retrieve HOTKEY pressed

		switch (controlID)
		{
		case CID_OK: // ok
			ApplySettings(tempSettings);
			SendMessage(GetWindow(m_hwnd, GW_OWNER), REFRESH_BRUSHES, 0, 0);
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
		case CID_CLICKTHROUGH: // Clickthrough checkbox
			tempSettings.clickthrough = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
			break;
		case COLOR_CTR_TIMER: // Any color control clicked
		case COLOR_CTR_SELECTED_TIMER:
		case COLOR_CTR_LAST_SECONDS:
		case COLOR_CTR_BACKGROUND:
		{
			// Open a Color Picker window
			if (pColorPicker->Window() == NULL)
			{
				pColorPicker->controlID = controlID; // Notify Color Picker who called it
				pColorPicker->pTempSettings = &tempSettings;

				if (!pColorPicker->Create(L"Color Picker", 850, 300, SIZE_COLORPICKER_WIDTH, SIZE_COLORPICKER_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, m_hwnd, 0, NULL)) {
					return;
				}
				ShowWindow(pColorPicker->Window(), SW_SHOW);
			}
			else
			{
				SetForegroundWindow(pColorPicker->Window());
			}
		}
		break;
		}
	}

public:
	ColorPickerWindow* pColorPicker; // Reference to a Color Picker window

	void ColorHandles(LPARAM lParam)
	{
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		// In case of invalid color index (to prevent index out of range)
		if (tempSettings.colors.timerColor > 24) {
			FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[0]);
			return;
		}

		// Apply saved color to the color controls in the setting's window
		switch (pDIS->CtlID)
		{
		case COLOR_CTR_TIMER:
			FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[tempSettings.colors.timerColor]);
			break;
		case COLOR_CTR_SELECTED_TIMER:
			FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[tempSettings.colors.selectedTimerColor]);
			break;
		case COLOR_CTR_LAST_SECONDS:
			FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[tempSettings.colors.lastSecondsColor]);
			break;
		case COLOR_CTR_BACKGROUND:
			FillRect(pDIS->hDC, &pDIS->rcItem, hBrushes[tempSettings.colors.backgroundColor]);
			break;
		}
	}

	// Overwrite of the message handler inherited from BaseWindow
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
		case WM_DRAWITEM: // Color controls
		{
			ColorHandles(lParam);
			return TRUE;
		}
		case WM_PAINT:
			DisplayBitmaps();
			break;
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
	ID2D1SolidColorBrush* pBrushTimer;
	ID2D1SolidColorBrush* pBrushSelectedTimer;
	ID2D1SolidColorBrush* pBrushLastSeconds;
	D2D1_COLOR_F backgroundColor;

	// Writing Resources
	IDWriteFactory* pWriteFactory;
	IDWriteTextFormat* pTextFormat;

private:
	Timer* activeTimer;

	BOOL mouseDown = false;
	int clickMousePos[2] = {0, 0};
	bool resizing = false;
	int dir = -1;
	int spaceOffset = 8;

	HRESULT CreateGraphicsResources()
	{
		HRESULT hr = S_OK;
		
		// Set up render target and brush
		if (pRenderTarget == nullptr)
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
				// Extract ColorF from HBRUSH
				settingsStruct settings = getSafeSettingsStruct();

				D2D1_COLOR_F timerColor = HBRUSHtoCOLORF(hBrushes[settings.colors.timerColor]);
				D2D1_COLOR_F selectedTimerColor = HBRUSHtoCOLORF(hBrushes[settings.colors.selectedTimerColor]);
				D2D1_COLOR_F lastSecondsColor = HBRUSHtoCOLORF(hBrushes[settings.colors.lastSecondsColor]);
				backgroundColor = HBRUSHtoCOLORF(hBrushes[settings.colors.backgroundColor]);

				// timer color brush
				hr = pRenderTarget->CreateSolidColorBrush(timerColor, &pBrushTimer);

				// selected timer color brush
				hr = pRenderTarget->CreateSolidColorBrush(selectedTimerColor, &pBrushSelectedTimer);

				// last seconds color brush
				hr = pRenderTarget->CreateSolidColorBrush(lastSecondsColor, &pBrushLastSeconds);
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
			if (pWriteFactory == nullptr) {
				hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory)
					, reinterpret_cast<IUnknown**>(&pWriteFactory));
			}

			if (SUCCEEDED(hr) && pTextFormat == nullptr)
			{
				// Set up text format
				static const WCHAR fontName[] = L"Sitka";
				static const int fontSize = 34;

				hr = pWriteFactory->CreateTextFormat(
					fontName,
					NULL,
					DWRITE_FONT_WEIGHT_BOLD,
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
					pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
				}
			}
		}

		return hr;
	}

	HRESULT ChangeFontSize(int fontSize)
	{
		SafeRelease(&pTextFormat);

		static const WCHAR fontName[] = L"Sitka";

		HRESULT hr = pWriteFactory->CreateTextFormat(
			fontName,
			NULL,
			DWRITE_FONT_WEIGHT_BOLD,
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
			pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		}

		return hr;
	}

	int GetLargestFontsizeFit()
	{
		IDWriteTextFormat* pTempTextFormat;
		IDWriteTextLayout* pTempTextLayout;

		int minSize = 10; int maxSize = 100;
		WCHAR fontFamily[] = L"Sitka";
		bool conditionMet = false;
		 
		wchar_t text[] = L"";

		// Iterate through options untill the largest possible font size is found
		while (!conditionMet)
		{
			HRESULT hr = pWriteFactory->CreateTextFormat(
				fontFamily,
				nullptr,
				pTextFormat->GetFontWeight(),
				pTextFormat->GetFontStyle(),
				pTextFormat->GetFontStretch(),
				maxSize,
				L"",
				&pTempTextFormat
			);

			if (FAILED(hr)) {
				PostQuitMessage(0);
			}

			hr = pWriteFactory->CreateTextLayout(
				text,
				(UINT32)wcslen(text),
				pTempTextFormat,
				winSize[0],
				winSize[1],
				&pTempTextLayout
			);

			if (FAILED(hr)) {
				PostQuitMessage(0);
			}

			// retrieve text size
			DWRITE_TEXT_METRICS layoutMetrics;
			hr = pTempTextLayout->GetMetrics(&layoutMetrics);

			if (FAILED(hr)) {
				PostQuitMessage(0);
			}

			if (layoutMetrics.width <= winSize[0] &&
				layoutMetrics.height <= winSize[1])
			{
				conditionMet = true;
			}
			else
			{
				maxSize--;
			}

			SafeRelease(&pTempTextFormat);
			SafeRelease(&pTempTextLayout);
		}

		return maxSize;
	}

	void DiscardGraphicsResources()
	{
		SafeRelease(&pRenderTarget);
		SafeRelease(&pBrushTimer);
		SafeRelease(&pBrushSelectedTimer);
		SafeRelease(&pBrushLastSeconds);
		SafeRelease(&pWriteFactory);
		SafeRelease(&pTextFormat);
	}

	void AdjustRendertargetSize()
	{
		D2D1_SIZE_U newSize = D2D1::SizeU(winSize[0], winSize[1]);
		pRenderTarget->Resize(newSize);
	}

	MousePos GetMouseDir(LPARAM lParam, RECT windowPos) {
		int currPos[2] = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		int width = windowPos.right - windowPos.left;
		int height = windowPos.bottom - windowPos.top;

		if (currPos[0] <= spaceOffset) // left
		{
			if (currPos[1] <= spaceOffset) { // top left
				return MousePos::topLeft;
			}
			else if (currPos[1] >= height - spaceOffset) { // bottom left
				SetCursor(LoadCursor(NULL, IDC_SIZENESW));
				return MousePos::bottomLeft;
			}
			else { // left
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				return MousePos::left;
			}
		}
		else if (currPos[0] >= width - spaceOffset) { // right
			if (currPos[1] <= spaceOffset) { // top right
				SetCursor(LoadCursor(NULL, IDC_SIZENESW));
				return MousePos::topRight;
			}
			else if (currPos[1] >= height - spaceOffset) { // bottom right
				SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				return MousePos::bottomRight;
			}
			else { // right
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				return MousePos::right;
			}
		}
		else if (currPos[1] >= height - spaceOffset) { // bottom
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			return MousePos::bottom;
		}
		else if (currPos[1] <= spaceOffset) // top
		{
			return MousePos::top;
		}

		return MousePos::none;
	}

	void HandlePainting()
	{
		HRESULT hr;
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);
		pRenderTarget->BeginDraw();

		// workaround to visible edges issue while transparent
		if (appSettings.optionTransparent) {
			pRenderTarget->Clear(D2D1::ColorF(0, 0, 0));
		}
		else {
			pRenderTarget->Clear(backgroundColor);
		}

		D2D1_RECT_F rect1 = D2D1::RectF(0, 0, winSize[0] / 2, winSize[1]);
		D2D1_RECT_F rect2 = D2D1::RectF(winSize[0] / 2, 0, winSize[0], winSize[1]);

		if (pWriteFactory != nullptr)
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
					pBrushTimer2 = pBrushLastSeconds;
				}
				else if (activeTimer == &timer2) {
					pBrushTimer2 = pBrushSelectedTimer;
				}
				else {
					pBrushTimer2 = pBrushTimer;
				}

				// Draw timers
				if (activeTimer == &timer1) {
					timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushSelectedTimer);
				}
				else {
					timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushTimer);
				}
				timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushTimer2);
			}
			else
			{
				timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushTimer);
				timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushTimer);
			}
		}

		hr = pRenderTarget->EndDraw();

		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hwnd, &ps);
	}

	void HandleMousemovement(LPARAM lParam) {
		// variables
		RECT windowPos;
		GetWindowRect(m_hwnd, &windowPos);
		int currPos[2] = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		int width = windowPos.right - windowPos.left;
		int height = windowPos.bottom - windowPos.top;		

		switch (GetMouseDir(lParam, windowPos))
		{
		case MousePos::topLeft:
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			break;
		case MousePos::top:
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			break;
		case MousePos::topRight:
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			break;
		case MousePos::right:
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			break;
		case MousePos::bottomRight:
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			break;
		case MousePos::bottom:
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			break;
		case MousePos::bottomLeft:
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			break;
		case MousePos::left:
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			break;
		case MousePos::none:
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		}

		if (dir == -1) // if not hovering resize area:
		{
			if (mouseDown && !resizing) // dragging the window
			{
				// drag window
				int xToMove = windowPos.left + (currPos[0] - clickMousePos[0]);
				int yToMove = windowPos.top + (currPos[1] - clickMousePos[1]);

				SetWindowPos(m_hwnd, NULL, xToMove, yToMove, winSize[0], winSize[1], 0);
			}
		}

		if (mouseDown && resizing) // if resizing
		{
			int newWidth = winSize[0]; int newHeight = winSize[1];
			int newX = windowPos.left; int newY = windowPos.top;

			// resizing logic
			switch (dir)
			{
			case 0: // horizontal
				if (currPos[1] <= spaceOffset) // resetting size from the top side
				{
					newY = windowPos.top + (currPos[1] - clickMousePos[1]);
					newHeight = windowPos.bottom - windowPos.top - (currPos[1] - clickMousePos[1]);
				}
				else // resetting size from the bottom side
				{
					newHeight = winSize[1] + currPos[1] - clickMousePos[1];
				}
				break;
			case 1: // vertical
			{
				if (currPos[0] <= spaceOffset) // resetting size from the left side
				{
					newX = windowPos.left + (currPos[0] - clickMousePos[0]);
					newWidth = (windowPos.right - windowPos.left) - (currPos[0] - clickMousePos[0]);
				}
				else // resetting size from the right side
				{
					newWidth = winSize[0] + currPos[0] - clickMousePos[0];
				}

			}
				break;
			case 2:
				if (currPos[0] <= spaceOffset) // resetting size from the left side
				{
					newX = windowPos.left + (currPos[0] - clickMousePos[0]);
					newWidth = (windowPos.right - windowPos.left) - (currPos[0] - clickMousePos[0]);
				}
				else // resetting size from the right side
				{
					newWidth = winSize[0] + currPos[0] - clickMousePos[0];
				}
				if (currPos[1] <= spaceOffset) // resetting size from the top side
				{
					newY = windowPos.top + (currPos[1] - clickMousePos[1]);
					newHeight = (windowPos.bottom - windowPos.top) - (currPos[1] - clickMousePos[1]);
				}
				else // resetting size from the bottom side
				{
					newHeight = winSize[1] + currPos[1] - clickMousePos[1];
				}
				break;
			}

			if (dir != -1) { // window was resized
				newWidth = max(25, min(700, newWidth));
				newHeight = max(25, min(700, newHeight));
				SetWindowPos(m_hwnd, NULL, newX, newY, newWidth, newHeight, 0);
			}
		}
	}

	D2D1_COLOR_F HBRUSHtoCOLORF(HBRUSH hBrush) {
		LOGBRUSH logBrush;

		// timer color
		GetObject(hBrush, sizeof(LOGBRUSH), &logBrush);
		COLORREF color = logBrush.lbColor; // Extract COLORREF
		return D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	}

	void RefreshBrushes()
	{
		// retrieve brushes colors
		settingsStruct settings = getSafeSettingsStruct();

		LOGBRUSH logBrush;

		// timer color
		pBrushTimer->SetColor(HBRUSHtoCOLORF(hBrushes[settings.colors.timerColor]));

		// selected timer color
		pBrushSelectedTimer->SetColor(HBRUSHtoCOLORF(hBrushes[settings.colors.selectedTimerColor]));

		// last seconds color
		pBrushLastSeconds->SetColor(HBRUSHtoCOLORF(hBrushes[settings.colors.lastSecondsColor]));

		// background color
		backgroundColor = HBRUSHtoCOLORF(hBrushes[settings.colors.backgroundColor]);
	}

public:
	Timer timer1 = Timer();
	Timer timer2 = Timer();

	SettingsWindow* pSettingsWindow;

	LPCWSTR ClassName() const { return L"Main Window"; }
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
	{
		RECT windowPos;
		GetWindowRect(m_hwnd, &windowPos);

		switch (wMsg)
		{
		case WM_CREATE:
		{
			if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory))) {
				return -1;
			}
			else {
				CreateGraphicsResources();
				CreateDeviceIndependentResources();
			}
			appSettings = getSafeSettingsStruct();
			appRunning = true;
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			appRunning = false;
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			mouseDown = true;
			clickMousePos[0] = GET_X_LPARAM(lParam); clickMousePos[1] = GET_Y_LPARAM(lParam);
			resizing = (GetCursor() != LoadCursor(NULL, IDC_ARROW));
			MousePos mPos = GetMouseDir(lParam, windowPos);
			
			// set drag direction according to mouse position
			if (mPos == MousePos::topRight || mPos == MousePos::topLeft ||
				mPos == MousePos::bottomLeft || mPos == MousePos::bottomRight)
			{
				dir = 2;
			}
			else if (mPos == MousePos::right || mPos == MousePos::left)
			{
				dir = 1;
			}
			else if (mPos == MousePos::top || mPos == MousePos::bottom)
			{
				dir = 0;
			}
			else
			{
				dir = -1;
			}

			SetCapture(m_hwnd);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			mouseDown = false;
			resizing = false;
			dir = -1;
			ReleaseCapture();

			// update winSize var after resizing
			if (windowPos.right - windowPos.left != winSize[0]) {
				winSize[0] = windowPos.right - windowPos.left;
				AdjustRendertargetSize();
				ChangeFontSize(GetLargestFontsizeFit());
			}
			if (windowPos.bottom - windowPos.top != winSize[1]) {
				winSize[1] = windowPos.bottom - windowPos.top;
				AdjustRendertargetSize();
				ChangeFontSize(GetLargestFontsizeFit());
			}
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			HandleMousemovement(lParam);
			return 0;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
			case MENU_SETTINGS:
				if (pSettingsWindow->Window() == NULL) // dont create multiple settings windows
				{
					// Create and show settings window
					if (!pSettingsWindow->Create(L"Settings - Version 1.1", 500, 200, SIZE_SETTINGS_WIDTH, SIZE_SETTINGS_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, m_hwnd, 0, 0, NULL)) {
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
		}
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
		case WM_SETCURSOR: 
			// disable default automatic cursor change (only manually set it)
			return 0;
		case REFRESH_BRUSHES:
			RefreshBrushes();
			break;
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

void controllerTest()
{
	XINPUT_STATE state = {};
	DWORD result = XInputGetState(0, &state);

	if (result == ERROR_SUCCESS) {
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) {
			bool test = true;
		}
	}
}

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

		controllerTest();
	}
}

// The hook procedure to listen for key strokes f1, f2 and f
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

	// Initialize brushes
	InitializeBrushes();

	// Create the main window
	MainWindow win;

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