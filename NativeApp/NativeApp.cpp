#include "stdafx.h"
// NativeApp.cpp : Defines the entry point for the application.
//


#include "NativeApp.h"
#include <winternl.h>
#include "NativeAppServiceConnection.h"
#include <string>
#include <mutex>
#include <memory>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND WINDOW;
int width = 400;
int height = 200;
std::condition_variable cond_var;
std::mutex m;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED))) {
		return 1;
	}

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NATIVEAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	hInst = hInstance;


	ConnectToService();
	std::unique_lock<std::mutex> lock(m);
	cond_var.wait(lock);

    return (int) 0;
}

PasswordWindow::PasswordWindow() {
}

int PasswordWindow::Run() {
	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);
	rect.left = (rect.right / 2) - (width / 2);
	rect.top = (rect.bottom / 2) - (height / 2);
	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUPWINDOW | WS_CAPTION, rect.left, rect.top, width, height, nullptr, nullptr, hInst, nullptr);

	if (!hWnd)
	{
		throw new std::exception();
	}

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_NATIVEAPP));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NATIVEAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDB_OK:
			{
				PasswordWindow *pw = (PasswordWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				GetWindowText(pw->GetEditWindow(), pw->GetPassword(), 256);
				PostQuitMessage(1);
				break;
			}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 20, 10, L"Password:", 9);
			PasswordWindow *pw = (PasswordWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			pw->SetEditWindow(CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_PASSWORD, 20, 30, 340, 30, hWnd, (HMENU)IDE_PASSWORD, NULL, NULL));
			CreateWindow(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | WS_BORDER, 140, 120, 100, 30, hWnd, (HMENU)IDB_OK, NULL, NULL);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
