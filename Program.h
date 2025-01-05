#pragma once
#include "MainWindow.h"

// Fields
extern MainWindow* pGlobalTimerWindow;

/*
@brief A method running an infinite loop, responsible for updating the timers.
*/
extern void AppLoop(MainWindow* win);

/*
@brief End the process of the program safely.
*/
extern void KillProgram();

/*
@brief The hook procedure to listen for key strokes of the active HOTKEYS.

@param nCode Has no effect in this function but is part of the syntax.

@param wParam Contains the state of the hit key.

@param lParam Contains the key that was hit.

@return Forwards to the next hook.
*/
LRESULT CALLBACK KBHook(int nCode, WPARAM wParam, LPARAM lParam);