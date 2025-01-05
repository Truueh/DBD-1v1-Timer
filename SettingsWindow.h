#pragma once
#include "ColorPickerWindow.h"
#include "globals.h"

// The class responsible for the settings window
class SettingsWindow : public BaseWindow<SettingsWindow>
{
private:
	// Fields
	settingsStruct tempSettings;
	HBITMAP mouseBitmap;
	HBITMAP controllerBitmap;

	// Methods
	
	/*
	@brief Calls required methods to initialize the settings window.
	*/
	void InitializeWindow();

	/*
	@brief Initializes the bitmaps for the settings window.
	*/
	void InitializeBitmaps();

	/*
	@brief Displays the bitmaps of the settings window. (controller & mouse images)
	*/
	void DisplayBitmaps();

	/*
	@brief Initializes all the text controls in the settings window.
	*/
	void InitializeTextControls();
	
	/*
	@brief Initializes all the interactable controls in the settings window.
	*/
	void InitializeButtonControls();

	/*
	@brief Creates a control and returns it.

	@param className The LPCWSTR representing the control's class name.

	@param controlName LPCWSTR representing the control's name.

	@param x Integer value representing the relative x position of the control on the window.

	@param y Integer value representing the relative y position of the control on the window.

	@param width Integer value to set the width of the control

	@param height Integer value to set the height of the control

	@param id Set an identifier for the control. Can be NULL.

	@param ADDITIONAL_STYLE Applies additional styles.

	@return The HWND control that was created.
	*/
	HWND CreateControl(LPCWSTR className, LPCWSTR controlName, int x, int y, int width, int height, int id = NULL, long ADDITIONAL_STYLE = 0);

	/*
	@brief Sets the "copyright" font to a given control.

	@param hControl The handle to the control to set the font of.
	*/
	void SetCopyrightFont(HWND hControl);

	/*
	@brief Method that handles interaction with controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void HandleControlCommand(LPARAM lParam);
	

public:
	ColorPickerWindow* pColorPicker; // Reference to a Color Picker window

	/*
	@brief Sets the preview colors of the color selection controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void ColorHandles(LPARAM lParam);

	/*
	@brief Implements an inherited method responsible for handling messages sent to the window.

	@param uMsg The message to handle. should be forwarded from a message handling method.

	@param wParam should be forwarded from a message handling method.

	@param lParam should be forwarded from a message handling method.

	@return LRESULT
	*/
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam);

	/*
	@brief Retrieves the window's class name.
	@return LPCWSTR representing the window's class name.
	*/
	LPCWSTR ClassName() const { return L"Settings Window"; }
};
