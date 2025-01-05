#include "MainWindow.h"
#include "enums.h"
#include <windowsx.h>
#include "globals.h"
#include "HelperFunctions.h"
#include "Program.h"

// Constructor
MainWindow::MainWindow()
{
}

// Destructor
MainWindow::~MainWindow() {
	MainWindow::DiscardGraphicsResources();
}

// Method defenitions
HRESULT MainWindow::CreateGraphicsResources()
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

HRESULT MainWindow::CreateDeviceIndependentResources()
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

HRESULT MainWindow::ChangeFontSize(int fontSize)
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

int MainWindow::GetLargestFontsizeFit()
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
			KillProgram();
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
			KillProgram();
		}

		// retrieve text size
		DWRITE_TEXT_METRICS layoutMetrics;
		hr = pTempTextLayout->GetMetrics(&layoutMetrics);

		if (FAILED(hr)) {
			KillProgram();
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

void MainWindow::DiscardGraphicsResources()
{
	SafeRelease(&pFactory);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pBrushTimer);
	SafeRelease(&pBrushSelectedTimer);
	SafeRelease(&pBrushLastSeconds);
	SafeRelease(&pWriteFactory);
	SafeRelease(&pTextFormat);
}

void MainWindow::AdjustRendertargetSize()
{
	D2D1_SIZE_U newSize = D2D1::SizeU(winSize[0], winSize[1]);
	pRenderTarget->Resize(newSize);
}

MousePos MainWindow::GetMouseDir(LPARAM lParam, RECT windowPos) {
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

void MainWindow::HandlePainting()
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

void MainWindow::HandleMousemovement(LPARAM lParam) {
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

D2D1_COLOR_F MainWindow::HBRUSHtoCOLORF(HBRUSH hBrush) {
	LOGBRUSH logBrush;

	// timer color
	GetObject(hBrush, sizeof(LOGBRUSH), &logBrush);
	COLORREF color = logBrush.lbColor; // Extract COLORREF
	return D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
}

void MainWindow::RefreshBrushes()
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

LRESULT MainWindow::HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
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
		KillProgram();
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
			KillProgram();
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

void MainWindow::HandleHotKey(int code)
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

void MainWindow::Draw() {
	HandlePainting();
}