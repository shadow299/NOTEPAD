#include <Windows.h>
#include <commdlg.h>

#define EDITED 1
#define UNTITLED TEXT("untitled")

void PopFileInitilize(HWND);
BOOL PopFileOpenDlg(HWND, PTSTR, PTSTR);
BOOL PopFileSaveDlg(HWND, PTSTR, PTSTR);
BOOL PopFileRead(HWND, PTSTR);
BOOL PopFileWrite(HWND, PTSTR);
