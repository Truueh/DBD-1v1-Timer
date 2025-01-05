#pragma once
#include "BaseWindow.h"

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
	
	/*
	@brief Creates and stores the squared controls that represent selectable colors.
	*/
	void InitializeColorButtons();

	/*
	@brief Calls required methods to initialize the settings window.
	*/
	void InitializeWindow();

	/*
	@brief Apply changes to the temporary struct the settings window is using
	*/
	void UpdateSettings();

	/*
	@brief Method that handles interaction with controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void HandleControlCommand(LPARAM lParam);

public:
	// public variables
	int controlID; // ID of the control that summoned the color picker window
	settingsStruct* pTempSettings; // Reference to the settings window's settings struct

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