#pragma once
#include "globals.h"

// Release pointers safely
template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// Methods

/*
@brief Safely retrieves the settingsStruct from the settings.json file.
		Handles errors in the settings.json file and resets it if need be.

@return The settingsStruct representing the saved settings from the settings.json file.
*/
settingsStruct getSafeSettingsStruct();

/*
@brief Writes the settings from the given settingsStruct to the settings.json file.

@param settings The settings struct to save to the file.
*/
void setSettingsStruct(settingsStruct settings);

/*
@brief Initial creation of the settings.json file.
*/
void createSettingsFile();

/*
@return Wether settings.json exists or not.
*/
bool settingsFileExists();

/*
@brief Set a specific control's font (title style).

@param hControl The control to change the font of.
*/
void SetTitleFont(HWND hControl);

/*
@brief Apply the default font to a specific window's child controls.

@param hWnd The handle to the window who's child controls will be affected.
*/
void SetControlsFont(HWND hWnd);

/*
@brief Save settings to local variable and json file (also apply temporary settings).

@param settings The settings to be applied.
*/
void ApplySettings(settingsStruct settings);

/*
@brief Load a bitmap resource.

@param bitmap The bitmap resource to load.

@return A handle to the bitmap loaded.
*/
HBITMAP LoadBitmapResource(int bitmap);

/*
@brief Initialize the colors that can be chosen (as brushes).
*/
void InitializeBrushes();

/*
@brief A procedure for windows of type "control", used to assign a font.

@param hControl The HWND control to assign the font to.

@param lParam An LPARAM containing the font to set for the hControl.

@return BOOL
*/
BOOL CALLBACK ControlProc(HWND hControl, LPARAM lParam);