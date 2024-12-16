#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include "helperFunctions.h"

using namespace std;

// Retrieve settings information from json file
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

// Set given information in json file
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

// Initial creation of the settings.json file
void createSettingsFile()
{
	ofstream file("settings.json");

	string json = "{\n   \"colors\" : \n   {\n      \"background\" : 20,\n      \"last seconds\" : 1,\n      \"selected timer\" : 6,\n      \"timer\" : 8\n   },\n   \"optionTransparent\" : false,\n   \"start\" : 70,\n   \"timer1\" : 112,\n   \"timer2\" : 113\n}";

	file << json;

	file.close();
}

// Return wether settings.json exists or not
bool settingsFileExists() {
	string name = "settings.json";

	ifstream f(name.c_str());
	return f.good();
}