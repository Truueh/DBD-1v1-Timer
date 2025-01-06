#include "SettingsWindow.h"
#include "HelperFunctions.h"
#include "BaseWindow.h"
#include "Program.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <exception>

// Methods

void SettingsWindow::InitializeWindow()
{
	// retrieve settings
	SettingsWindow::tempSettings = getSafeSettingsStruct();

	// Set up text
	InitializeTextControls();

	// Set up buttons and hotkeys
	InitializeButtonControls();

	// Set up bitmaps
	InitializeBitmaps();
}

void SettingsWindow::InitializeBitmaps()
{
	mouseBitmap = LoadBitmapResource(IDB_MOUSE);
	controllerBitmap = LoadBitmapResource(IDB_CONTROLLER);
}

void SettingsWindow::DisplayBitmaps()
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

void SettingsWindow::InitializeTextControls()
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

void SettingsWindow::InitializeButtonControls()
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

	// Apply font
	SetControlsFont(m_hwnd);

	// Apply current settings
	SendMessage(hHotkeyStart, HKM_SETHOTKEY, appSettings.startKey, 0);
	SendMessage(hHotkeyTimer1, HKM_SETHOTKEY, appSettings.timer1Key, 0);
	SendMessage(hHotkeyTimer2, HKM_SETHOTKEY, appSettings.timer2Key, 0);
	SendMessage(hCbTransparentBg, BM_SETCHECK, appSettings.optionTransparent, 0);
	SendMessage(hCbClickthrough, BM_SETCHECK, appSettings.clickthrough, 0);
}

HWND SettingsWindow::CreateControl(LPCWSTR className, LPCWSTR controlName, int x, int y, int width, int height, int id, long ADDITIONAL_STYLE) {
	return CreateWindowEx(0, className, controlName, WS_VISIBLE | WS_CHILDWINDOW | ADDITIONAL_STYLE, x, y, width, height, m_hwnd, (HMENU)id, NULL, NULL);
}

void SettingsWindow::SetCopyrightFont(HWND hControl)
{
	HFONT hFont = CreateFont(
		14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void SettingsWindow::HandleControlCommand(LPARAM lParam)
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

void SettingsWindow::ColorHandles(LPARAM lParam)
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

LRESULT SettingsWindow::HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	try
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
	}
	catch (const std::exception e)
	{
		KillProgram();
	}
	return DefWindowProc(Window(), wMsg, wParam, lParam);
}