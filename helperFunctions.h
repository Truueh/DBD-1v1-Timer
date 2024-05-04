// HWND Control IDs
#define CID_OK					1
#define CID_CANCEL				2
#define CID_START				3
#define CID_TIMER1				4
#define CID_TIMER2				5
#define MENU_QUIT				1
#define MENU_SETTINGS			0

struct settingsStruct
{
	int startKey;
	int timer1Key;
	int timer2Key;
};

// HelperFunctions.cpp functions
settingsStruct getSettingsStruct();

void setSettingsStruct(settingsStruct settings);