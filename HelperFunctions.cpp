#include <json/value.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include "helperFunctions.h"

using namespace std;

// Retrieve settings information from json file
settingsStruct getSettingsStruct()
{
	ifstream file("settings.json");
	Json::Value actualJson;
	Json::Reader reader;
	settingsStruct settings;
	
	reader.parse(file, actualJson);

	settings.startKey = actualJson["start"].asInt();
	settings.timer1Key = actualJson["timer1"].asInt();
	settings.timer2Key = actualJson["timer2"].asInt();
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

	// Write to file
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";

	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	std::ofstream outputFileStream("settings.json");
	writer->write(settingsJson, &outputFileStream);
}