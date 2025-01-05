#include "HelperFunctions.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

using namespace std;

// Methods
settingsStruct getSafeSettingsStruct()
{
	ifstream file("settings.json");
	Json::Value actualJson;
	Json::Reader reader;
	settingsStruct settings;
	
	reader.parse(file, actualJson);

	// hotkeys
	if (actualJson["start"].isInt() && actualJson["timer1"].isInt() && actualJson["timer2"].isInt())
	{
		settings.startKey = actualJson["start"].asInt();
		settings.timer1Key = actualJson["timer1"].asInt();
		settings.timer2Key = actualJson["timer2"].asInt();
	}
	else
	{
		settings.startKey = 70;
		settings.timer1Key = 112;
		settings.timer2Key = 113;
	}

	// options
	if (actualJson["optionTransparent"].isBool()) {
		settings.optionTransparent = actualJson["optionTransparent"].asBool();
	}
	else {
		settings.optionTransparent = false;
	}
	settings.clickthrough = false;

	// colors
	Json::Value colors = actualJson["colors"];
	if (colors["timer"].isInt() && colors["selected timer"].isInt()
		&& colors["last seconds"].isInt() && colors["background"].isInt())
	{
		settings.colors.timerColor = colors["timer"].asInt();
		settings.colors.selectedTimerColor = colors["selected timer"].asInt();
		settings.colors.lastSecondsColor = colors["last seconds"].asInt();
		settings.colors.backgroundColor = colors["background"].asInt();

		if (settings.colors.timerColor > 24 || settings.colors.selectedTimerColor > 24 ||
			settings.colors.lastSecondsColor > 24 || settings.colors.backgroundColor > 24)
		{
			settings.colors.timerColor = 8;
			settings.colors.selectedTimerColor = 6;
			settings.colors.lastSecondsColor = 1;
			settings.colors.backgroundColor = 20;
		}
	}
	else 
	{
		settings.colors.timerColor = 8;
		settings.colors.selectedTimerColor = 6;
		settings.colors.lastSecondsColor = 1;
		settings.colors.backgroundColor = 20;
	}


	return settings;
}

void setSettingsStruct(settingsStruct settings)
{
	ifstream file("settings.json");

	// Retrieve existing settings
	Json::Reader reader; 
	Json::Value settingsJson;
	reader.parse(file, settingsJson);

	// Change json obj
	settingsJson["start"] = settings.startKey;
	settingsJson["timer1"] = settings.timer1Key;
	settingsJson["timer2"] = settings.timer2Key;
	settingsJson["optionTransparent"] = settings.optionTransparent;
	settingsJson["colors"]["timer"] = settings.colors.timerColor;
	settingsJson["colors"]["selected timer"] = settings.colors.selectedTimerColor;
	settingsJson["colors"]["last seconds"] = settings.colors.lastSecondsColor;
	settingsJson["colors"]["background"] = settings.colors.backgroundColor;

	// Write to file
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";

	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	std::ofstream outputFileStream("settings.json");
	writer->write(settingsJson, &outputFileStream);
}

void createSettingsFile()
{
	ofstream file("settings.json");

	string json = "{\n   \"colors\" : \n   {\n      \"background\" : 20,\n      \"last seconds\" : 1,\n      \"selected timer\" : 6,\n      \"timer\" : 8\n   },\n   \"optionTransparent\" : false,\n   \"start\" : 70,\n   \"timer1\" : 112,\n   \"timer2\" : 113\n}";

	file << json;

	file.close();
}

bool settingsFileExists() {
	string name = "settings.json";

	ifstream f(name.c_str());
	return f.good();
}

BOOL CALLBACK ControlProc(HWND hControl, LPARAM lParam)
{
	HFONT hFont = (HFONT)lParam;
	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);

	return true;
}

void SetTitleFont(HWND hControl)
{
	HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

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

HBITMAP LoadBitmapResource(int bitmap) {
	HBITMAP hBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmap));
	if (!hBitmap) {
		MessageBox(NULL, L"Failed to load bitmap!", L"Error", MB_OK);
	}

	return hBitmap;
}

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