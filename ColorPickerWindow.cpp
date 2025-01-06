#include <Windows.h>
#include <CommCtrl.h>
#include "globals.h"
#include "HelperFunctions.h"
#include "ColorPickerWindow.h"
#include "Program.h"
#include <exception>

// Methods
// Create and store the squared controls that represent selectable colors
void ColorPickerWindow::InitializeColorButtons()
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

void ColorPickerWindow::InitializeWindow()
{
	InitializeColorButtons();

	// Initialize exit controls
	HWND hwndOKButton = CreateWindowEx(0, WC_BUTTON, L"OK", WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 150, SIZE_COLORPICKER_HEIGHT - 70, 50, 25, m_hwnd, (HMENU)CID_OK, NULL, NULL);
	HWND hwndCancelButton = CreateWindowEx(0, WC_BUTTON, L"CANCEL", WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 90, SIZE_COLORPICKER_HEIGHT - 70, 70, 25, m_hwnd, (HMENU)CID_CANCEL, NULL, NULL);

	SetControlsFont(m_hwnd);
}

void ColorPickerWindow::UpdateSettings()
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

void ColorPickerWindow::HandleControlCommand(LPARAM lParam)
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

LRESULT ColorPickerWindow::HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
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
	}
	catch (const std::exception e)
	{
		KillProgram();
	}
	return DefWindowProc(Window(), wMsg, wParam, lParam);
}