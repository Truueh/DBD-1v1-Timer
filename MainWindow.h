#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include "BaseWindow.h"
#include "Timer.h"
#include "SettingsWindow.h"

// The class responsible for the main window of the app
class MainWindow : public BaseWindow<MainWindow>
{
private:
	// Resources
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrushTimer;
	ID2D1SolidColorBrush* pBrushSelectedTimer;
	ID2D1SolidColorBrush* pBrushLastSeconds;
	D2D1_COLOR_F backgroundColor;

	// Writing Resources
	IDWriteFactory* pWriteFactory;
	IDWriteTextFormat* pTextFormat;

	// Fields
	Timer* activeTimer;
	BOOL mouseDown = false;
	int clickMousePos[2] = { 0, 0 };
	bool resizing = false;
	int dir = -1;
	int spaceOffset = 8;
	
	// Methods

	/*
	@brief Creates the graphic resources for the main window.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT CreateGraphicsResources();

	/*
	@brief Creates the device independent resources for the main window.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT CreateDeviceIndependentResources();

	/*
	@brief Set a new font size for the timers.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT ChangeFontSize(int fontSize);

	/*
	@return The largest font size that can fit the window in it's current proportions.
	*/
	int GetLargestFontsizeFit();

	/*
	@brief Dispose of graphic resources.
	*/
	void DiscardGraphicsResources();

	/*
	@brief Adjust the render target's size after the window has been resized.
	*/
	void AdjustRendertargetSize();

	/*
	@brief Get the MousePos enum value of the mouse's direction.

	@param lParam Contains information on the mouse's state. Should be forwarded from a message handler.

	@param windowPos A RECT representing the main window's current proportions.

	@return The MousePos enum value of the mouse's direction.
	*/
	MousePos GetMouseDir(LPARAM lParam, RECT windowPos);

	/*
	@brief The method responsible for drawing to the main window and invoking the timer to draw to it aswell.
	*/
	void HandlePainting();

	/*
	@brief Handles mouse movements, including changing the cursor according to it's position in the window, 
	and detecting clicks to apply resize logic.

	@param lParam Contains information on the mouse's state and position. 
	This argument should be forwarded to this function by the message handler.
	*/
	void HandleMousemovement(LPARAM lParam);

	/*
	@brief Get the COLORF value of an HBRUSH type.

	@param hBrush The HBRUSH to retrieve a COLORF from.

	@return D2D1_COLOR_F value of the given HBRUSH.
	*/
	D2D1_COLOR_F HBRUSHtoCOLORF(HBRUSH hBrush);

	/*
	@brief Retrieve and apply the colors from the settings file to the timer brushes.
	*/
	void RefreshBrushes();

public:
	// Public fields
	Timer timer1 = Timer();
	Timer timer2 = Timer();
	SettingsWindow* pSettingsWindow;

	// Constructor
	MainWindow();

	// Destructor
	~MainWindow();

	// Methods

	/*
	@brief Retrieves the window's class name.
	@return LPCWSTR representing the window's class name.
	*/
	LPCWSTR ClassName() const { return L"Main Window"; }

	/*
	@brief Implements an inherited method responsible for handling messages sent to the window.

	@param uMsg The message to handle. should be forwarded from a message handling method.

	@param wParam should be forwarded from a message handling method.

	@param lParam should be forwarded from a message handling method.

	@return LRESULT
	*/
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam);

	/*
	@brief Handle hotkey inputs from the user.

	@param code The code of the hotkey that the user hit.
	*/
	void HandleHotKey(int code);

	/*
	@brief The function that is called when a WM_PAINT event is registered to the window. This method forwards the task to HandlePainting().
	*/
	void Draw();
};