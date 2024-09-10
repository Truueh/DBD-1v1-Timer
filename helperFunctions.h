// HWND Control IDs
#define CID_OK					1
#define CID_CANCEL				2
#define CID_START				3
#define CID_TIMER1				4
#define CID_TIMER2				5
#define CID_TRANSPARENT_CB		6
#define CID_CLICKTHROUGH		7
#define MENU_QUIT				1
#define MENU_SETTINGS			0
#define KEY_START				0
#define KEY_TIMER1				1
#define KEY_TIMER2				2
#define OPTION_TRANSPARENT		3
#define OPTION_CLICKTHROUGH		4

struct settingsStruct
{
	int startKey;
	int timer1Key;
	int timer2Key;
	bool optionTransparent;
	bool clickthrough;
};

// HelperFunctions.cpp functions
settingsStruct getSettingsStruct();

void setSettingsStruct(settingsStruct settings);

void createSettingsFile();

bool settingsFileExists();