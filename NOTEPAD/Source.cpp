#include <Windows.h>
#include <commdlg.h>
#include "resource.h"

#define EDITED 1
#define UNTITLED TEXT("untitled")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

//difining some useful functions

//<<-----Functions for FILE menu ------->>
void PopFileInitilize(HWND);
BOOL PopFileOpenDlg(HWND, PTSTR, PTSTR);
BOOL PopFileSaveDlg(HWND, PTSTR, PTSTR);
BOOL PopFileRead(HWND, PTSTR);
BOOL PopFileWrite(HWND, PTSTR);

//<<------Functions for FIND menu--------->>
HWND PopFindFindDlg(HWND);
HWND PopFindReplaceDlg(HWND);
BOOL PopFindFindText(HWND, int*, LPFINDREPLACE);
BOOL PopFindReplaceText(HWND, int*, LPFINDREPLACE);
BOOL PopFindNextText(HWND, int*);
BOOL PopFindValidFind(void);

//<<-------Function for FONT menu----------->>
void PopFontInitilize(HWND);
BOOL PopFontChooseFont(HWND);
void PopFontSetFont(HWND);
void PopFontDeinitilize(void);

//<<-------Function for Print menu---------->>
BOOL PopPrintPrintFile(HINSTANCE, HWND, HWND, PTSTR);


//Global variables
static HWND hDlgModelless;
static TCHAR szAppName[] = TEXT("POP-PAD");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrivInstance, PSTR szCmdLine, int iCmdShow) {
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	HACCEL hAccel;

	wndclass.style = CS_VREDRAW | CS_HREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIconW(NULL, IDI_WINLOGO);
	wndclass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	//register class
	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, TEXT("This program requires windows!!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(
		szAppName,
		TEXT("Hello win32 API"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		szCmdLine);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (hDlgModelless == 0 || !IsDialogMessage(hDlgModelless, &msg)) {
			if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return msg.wParam;
}

void DoCaption(HWND hwnd, TCHAR* szTitleName) {
	TCHAR szCaption[64 + MAX_PATH];
	wsprintf(szCaption, TEXT("%s - %s"), szAppName, szTitleName[0] ? szTitleName : UNTITLED);
	SetWindowText(hwnd, szCaption);
}

void OkMessage(HWND hwnd, TCHAR* szMessage, TCHAR* szTitleName) {
	TCHAR szBuffer[64 + MAX_PATH];
	wsprintf(szBuffer, szMessage, szTitleName[0] ? szTitleName : UNTITLED);
	MessageBox(hwnd, szBuffer, szAppName, MB_OK | MB_ICONEXCLAMATION);
}

short AskAboutSave(HWND hwnd, TCHAR* szTitleName) {
	TCHAR szBuffer[64 + MAX_PATH];
	int iReturn;
	wsprintf(szBuffer, TEXT("Save current changes in %s?"), szTitleName[0] ? szTitleName : UNTITLED);

	iReturn = MessageBox(hwnd, szBuffer, szAppName, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (iReturn == IDYES) {
		if (!SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0)) {
			iReturn = IDCANCEL;
		}
	}
	return iReturn;
}

//handling window message
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	LPCWSTR name = L"CREATED";

	switch (message) {
	case WM_CREATE:
		MessageBox(NULL, name, name, MB_OK);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		DrawText(hdc, TEXT("Hello Windows 10"), -1, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}