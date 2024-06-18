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

// Initial creation of the settings.json file
void createSettingsFile()
{
	ofstream file("settings.json");

	string json = "{\n\"start\": 70,\n\"timer1\": 112,\n\"timer2\": 113\n}";

	file << json;

	file.close();
}

// Return wether settings.json exists or not
bool settingsFileExists() {
	string name = "settings.json";

	ifstream f(name.c_str());
	return f.good();
}