#pragma once


#include "stdafx.h"
#include "resource.h"

class PasswordWindow {
	HWND hWnd;
	HWND hEditWindow;
	WCHAR password[256];

public:
	PasswordWindow();
	int Run();
	void SetEditWindow(HWND hw) { hEditWindow = hw;  }
	HWND GetEditWindow() { return hEditWindow; }
	WCHAR* GetPassword() { return password; }
};