#include <Windows.h>
#include <commdlg.h>

#define EDITID 1
#define UNTITLED TEXT("untitled")

void PopFontInitilize(HWND);
BOOL PopFontChooseFont(HWND);
void PopFontSetFont(HWND);
void PopFontDeinitilize(void);