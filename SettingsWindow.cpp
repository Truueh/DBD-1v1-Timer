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
	int mouseX = SIZE_SETTINGS_WIDTH / 2 + 45; int mouseY = 35;
	int controllerX = mouseX + 75; int controllerY = 35;
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
	HWND hwndTitleOptions = CreateControl(WC_STATIC, L"Options", SIZE_SETTINGS_WIDTH / 2 - 40, yOffset * 6 - 20, 80, 40);
	HWND hwndTitleColors = CreateControl(WC_STATIC, L"Colors", SIZE_SETTINGS_WIDTH / 2 - 40, yOffset * 10 - 20, 60, 40);

	// Hotkey titles
	HWND hwndTextStart = CreateControl(WC_STATIC, L"Start / Stop / Reset", xTitle, yOffset * 2, 150, 40);
	HWND hwndTextTimer1 = CreateControl(WC_STATIC, L"Timer 1", xTitle, yOffset * 3, 150, 20);
	HWND hwndTextTimer2 = CreateControl(WC_STATIC, L"Timer 2", xTitle, yOffset * 4, 150, 20);

	// Checkbox titles
	HWND hwndTextTransparentBackground = CreateControl(WC_STATIC, L"Transparent Background", xTitle, yOffset * 7 - 20, 150, 20);
	HWND hwndTextCheckboxClickthrough = CreateControl(WC_STATIC, L"Clickthrough (resets when app is closed)", xTitle, yOffset * 8 - 20, 150, 40);

	// Break lines
	HWND hwndBreakLine1 = CreateControl(WC_STATIC, NULL, 15, yOffset * 5, SIZE_SETTINGS_WIDTH - 40, 5, NULL, SS_ETCHEDHORZ);
	HWND hwndBreakLine2 = CreateControl(WC_STATIC, NULL, 15, yOffset * 9, SIZE_SETTINGS_WIDTH - 40, 5, NULL, SS_ETCHEDHORZ);

	// Color options names
	HWND hwndTextColorTimer = CreateControl(WC_STATIC, L"Timer", xTitle, yOffset * 11 - 20, 150, 40);
	HWND hwndTextColorSelectedTimer = CreateControl(WC_STATIC, L"Selected Timer", xTitle, yOffset * 12 - 20, 150, 40);
	HWND hwndTextColorWinCon = CreateControl(WC_STATIC, L"Last 20 Seconds", xTitle, yOffset * 13 - 20, 150, 40);
	HWND hwndTextColorBackground = CreateControl(WC_STATIC, L"Background", xTitle, yOffset * 14 - 20, 150, 40);

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

	int xHotkey = SIZE_SETTINGS_WIDTH / 2 + 20;
	int xHotkeyCon = xHotkey + 80;
	int xColorButton = xHotkeyCon - 50;
	int widthHotkey = 70;
	int heightHotkey = 20;
	int sizeCheckbox = 40;
	int xCheckbox = SIZE_SETTINGS_WIDTH - 70;

	HWND hotkeys[6];
	HWND colorButtons[4];

	// Hotkeys
		// Mouse
	hotkeys[0] = CreateControl(WC_BUTTON, L"", xHotkey, yOffset * 2, widthHotkey, heightHotkey, CID_START, BS_FLAT); // Start key
	hotkeys[1] = CreateControl(WC_BUTTON, L"", xHotkey, yOffset * 3, widthHotkey, heightHotkey, CID_TIMER1, BS_FLAT); // Timer 1 key
	hotkeys[2] = CreateControl(WC_BUTTON, L"", xHotkey, yOffset * 4, widthHotkey, heightHotkey, CID_TIMER2, BS_FLAT); // Timer 2 key

		// Controller
	hotkeys[3] = CreateControl(WC_BUTTON, L"test", xHotkeyCon, yOffset * 2, widthHotkey, heightHotkey, 0, BS_FLAT); // Start key
	hotkeys[4] = CreateControl(WC_BUTTON, L"test", xHotkeyCon, yOffset * 3, widthHotkey, heightHotkey, 0, BS_FLAT); // Timer 1 key
	hotkeys[5] = CreateControl(WC_BUTTON, L"test", xHotkeyCon, yOffset * 4, widthHotkey, heightHotkey, 0, BS_FLAT); // Timer 2 key

	// Checkbox buttons
	HWND hCbTransparentBg = CreateControl(WC_BUTTON, L"", xCheckbox, yOffset * 7 - 30, sizeCheckbox, sizeCheckbox, CID_TRANSPARENT_CB, BS_CHECKBOX | BS_AUTOCHECKBOX);
	HWND hCbClickthrough = CreateControl(WC_BUTTON, L"", xCheckbox, yOffset * 8 - 30, sizeCheckbox, sizeCheckbox, CID_CLICKTHROUGH, BS_CHECKBOX | BS_AUTOCHECKBOX);

	// Color buttons
	colorButtons[0] = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 11 - 20, 100, 15, COLOR_CTR_TIMER, BS_OWNERDRAW);
	colorButtons[1] = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 12 - 20, 100, 15, COLOR_CTR_SELECTED_TIMER, BS_OWNERDRAW);
	colorButtons[2] = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 13 - 20, 100, 15, COLOR_CTR_LAST_SECONDS, BS_OWNERDRAW);
	colorButtons[3] = CreateControl(WC_BUTTON, L"", xColorButton, yOffset * 14 - 20, 100, 15, COLOR_CTR_BACKGROUND, BS_OWNERDRAW);

	// Initialize exit controls
	HWND hwndOKButton = CreateControl(WC_BUTTON, L"OK", SIZE_SETTINGS_WIDTH - 160, SIZE_SETTINGS_HEIGHT - 80, 50, 25, CID_OK);
	HWND hwndCancelButton = CreateControl(WC_BUTTON, L"CANCEL", SIZE_SETTINGS_WIDTH - 100, SIZE_SETTINGS_HEIGHT - 80, 70, 25, CID_CANCEL);

	// Apply font
	SetControlsFont(m_hwnd);
	
	// Apply currently set hotkeys
	for (HWND hCtrl : hotkeys) {
		ApplyHotkeySavedKey(hCtrl);
	}

	// Apply currently set options
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

	// Clicked control
	switch (controlID) {
	// OK
	case CID_OK:
		ApplySettings(tempSettings);
		SendMessage(GetWindow(m_hwnd, GW_OWNER), REFRESH_BRUSHES, 0, 0);
		DestroyWindow(m_hwnd);
		break;
		
	// Cancel
	case CID_CANCEL:
		DestroyWindow(m_hwnd);
		break;

	// Any hotkey control clicked
	case CID_START:
	case CID_TIMER1:
	case CID_TIMER2:
	{
		if (hActiveControl) { // Reset currently selected control (before this control was clicked)
			ApplyHotkeySavedKey(hActiveControl);
			hActiveControl = nullptr;
		}

		SetFocus(m_hwnd);
		hActiveControl = hwndCtrl;
		SetWindowText(hActiveControl, L"...");
		break;
	}

	// Transparent background checkbox
	case CID_TRANSPARENT_CB: 
		tempSettings.optionTransparent = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
		break;

	// Clickthrough checkbox
	case CID_CLICKTHROUGH:
		tempSettings.clickthrough = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
		break;

	// Any color control clicked
	case COLOR_CTR_TIMER:
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

void SettingsWindow::ApplyTempHotkey(UINT key) {
	int controlID = GetDlgCtrlID(hActiveControl); // retrieve control ID

	switch (controlID) {
	case CID_START:
		tempSettings.startKey = key;
		break;
	case CID_TIMER1:
		tempSettings.timer1Key = key;
		break;
	case CID_TIMER2:
		tempSettings.timer2Key = key;
		break;
	}

	SetWindowText(hActiveControl, KeyboardMap[key]);
	hActiveControl = nullptr;
}

void SettingsWindow::ApplyHotkeySavedKey(HWND hCtrl) {
	int controlID = GetDlgCtrlID(hCtrl); // retrieve control ID

	switch (controlID) {
	case CID_START:
		if (KeyboardMap.count(tempSettings.startKey)) { // Verify key exists
			SetWindowText(hCtrl, KeyboardMap[tempSettings.startKey]);
		}
		break;
	case CID_TIMER1:
		if (KeyboardMap.count(tempSettings.timer1Key)) { // Verify key exists
			SetWindowText(hCtrl, KeyboardMap[tempSettings.timer1Key]);
		}
		break;
	case CID_TIMER2:
		if (KeyboardMap.count(tempSettings.timer2Key)) { // Verify key exists
			SetWindowText(hCtrl, KeyboardMap[tempSettings.timer2Key]);
		}
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
			//TEST
		case WM_LBUTTONDOWN:
			if (hActiveControl) {
				ApplyHotkeySavedKey(hActiveControl);
				hActiveControl = nullptr;
			}
			break;
		case WM_KEYDOWN:
			if (hActiveControl) {
				UINT key = (UINT)wParam;
				ApplyTempHotkey(key);
			}
			break;
		case WM_SYSKEYDOWN: // Specifically ALT key
			if (hActiveControl && (UINT)wParam == VK_MENU) {
				UINT key = (UINT)wParam;
				ApplyTempHotkey(key);
			}
			break;
		}
	}
	catch (const std::exception e)
	{
		KillProgram();
	}
	return DefWindowProc(Window(), wMsg, wParam, lParam);
}