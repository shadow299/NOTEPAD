#include <Windows.h>
#include <commdlg.h>
#include "resource.h"
#include "filemenuoptions.h"
#include "findmenuoptions.h"
#include "fontmenuoptions.h"



#define EDITID 1
#define UNTITLED TEXT("untitled")


//Global variables
BOOL bUserAbort;
HWND hDlgPrint;
static HWND hDlgModelless;
static TCHAR szAppName[] = TEXT("POP-PAD");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK PrintDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		bUserAbort = TRUE;
		EnableWindow(GetParent(hdlg), TRUE);
		DestroyWindow(hdlg);
		hDlgPrint = NULL;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK AbortProc(HDC hdcprn, int iCode) {
	MSG msg;

	while (!bUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!hDlgPrint || !IsDialogMessage(hDlgPrint, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return !bUserAbort;
}

BOOL PopPrintPrintFile(HINSTANCE hInst, HWND hwnd, HWND hwndEdit, PTSTR pstrTitleName) {
	static DOCINFO di = { sizeof(DOCINFO) };
	static PRINTDLG pd;
	BOOL bSuccess;
	int yChar, iCharPerLines, iLinesPerPage, iTotalLines, iTotalPages, iPage, iLine, iLIneNum;
	PTSTR pstrBuffer;
	TCHAR szJobName[64 + MAX_PATH];
	TEXTMETRIC tm;
	WORD iColCopy, iNoiColCopy;

	//invoke print common dialog box

	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwnd;
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;
	pd.hDC = NULL;
	pd.Flags = PD_ALLPAGES | PD_COLLATE | PD_RETURNDC | PD_NOSELECTION;
	pd.nFromPage = 0;
	pd.nToPage = 0;
	pd.nMinPage = 0;
	pd.nMaxPage = 0;
	pd.nCopies = 1;
	pd.hInstance = NULL;
	pd.lCustData = 0L;
	pd.lpfnPrintHook = NULL;
	pd.lpfnSetupHook = NULL;
	pd.lpPrintTemplateName = NULL;
	pd.lpSetupTemplateName = NULL;
	pd.hPrintTemplate = NULL;
	pd.hSetupTemplate = NULL;

	if (!PrintDlg(&pd)) {
		return TRUE;
	}

	if (0 == (iTotalLines = SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0))) {
		return TRUE;
	}

	GetTextMetrics(pd.hDC, &tm);
	yChar = tm.tmHeight + tm.tmExternalLeading;
	iCharPerLines = GetDeviceCaps(pd.hDC, HORZRES) / tm.tmAveCharWidth;
	iLinesPerPage = GetDeviceCaps(pd.hDC, VERTRES) / yChar;
	iTotalPages = (iTotalLines + iLinesPerPage - 1) / iLinesPerPage;

	//Allocate buffer for each line

	pstrBuffer = (PTSTR)malloc(sizeof(TCHAR) * (iCharPerLines + 1));

	//diaplay printing dialog box

	EnableWindow(hwnd, FALSE);
	bSuccess = TRUE;
	bUserAbort = FALSE;

	hDlgPrint = CreateDialog(hInst, MAKEINTRESOURCE(PRINT_DLG), hwnd, PrintDlgProc);
	SetDlgItemText(hDlgPrint, IDC_FILENAME, pstrTitleName);

	SetAbortProc(pd.hDC, AbortProc);

	//start the doc
	GetWindowText(hwnd, szJobName, sizeof(szJobName));
	di.lpszDocName = szJobName;
	if (StartDoc(pd.hDC, &di) > 0) {
		for (iColCopy = 0; iColCopy < ((WORD)pd.Flags & PD_COLLATE ? pd.nCopies : 1); iColCopy++) {
			for (iPage = 0; iPage < iTotalPages; iPage++) {
				for (iNoiColCopy = 0; iNoiColCopy < ((WORD)pd.Flags & PD_COLLATE ? pd.nCopies : 1); iNoiColCopy++) {
					//start the page
					if (StartPage(pd.hDC) < 0) {
						bSuccess = FALSE;
						break;
					}

					//for each page print the line
					for (iLine = 0; iLine < iLinesPerPage; iLine++) {
						iLIneNum = iLinesPerPage * iPage + iLine;
						if (iLIneNum > iTotalLines) {
							break;
						}
						*(int*)pstrBuffer = iCharPerLines;
						TextOut(pd.hDC, 0, yChar * iLine, pstrBuffer, (int)SendMessage(hwndEdit, EM_GETLINE, (WPARAM)iLIneNum, (LPARAM)pstrBuffer));
					}
					if (EndPage(pd.hDC) < 0) {
						bSuccess = FALSE;
						break;
					}
					if (bUserAbort)
						break;
				}
				if (!bSuccess || bUserAbort)
					break;
			}
			if (!bSuccess || bUserAbort)
				break;
		}
	}
	else
		bSuccess = FALSE;

	if (bSuccess)
		EndDoc(pd.hDC);

	if (!bUserAbort) {
		EnableWindow(hwnd, TRUE);
		DestroyWindow(hDlgPrint);
	}
	free(pstrBuffer);
	DeleteDC(pd.hDC);
	return bSuccess && !bUserAbort;

}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrivInstance, PSTR szCmdLine, int iCmdShow) {
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	HACCEL hAccel;
	HMENU hMenu;

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

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));

	hwnd = CreateWindow(
		szAppName,
		TEXT("POP-PAD"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		hMenu,
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
	static BOOL bNeedSave = FALSE;
	static HINSTANCE hInst;
	static HWND hwndEdit;
	static int iOffset;
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static UINT messageFindReplace;
	int iSelBeg, iSelEnd, iEnable;
	LPFINDREPLACE pfr;
	

	switch (message) {
	case WM_CREATE:
		hInst = ((LPCREATESTRUCT)lParam)->hInstance;

		//creation of edit child window
		hwndEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hwnd, (HMENU)EDITID, hInst, NULL);
		SendMessage(hwndEdit, EM_LIMITTEXT, 32000, 0L);

		//initilize common dialog box functions
		PopFileInitilize(hwnd);
		PopFontInitilize(hwndEdit);

		messageFindReplace = RegisterWindowMessage(FINDMSGSTRING);

		return 0;

	case WM_SETFOCUS:
		SetFocus(hwndEdit);
		return 0;

	case WM_SIZE:
		MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	
	case WM_INITMENUPOPUP:
		switch (lParam) {
		case 1:
			//edit menu
			//enable undo if edit menu is ready to do it
			EnableMenuItem((HMENU)wParam, ID_EDIT_UNDO, SendMessage(hwndEdit, EM_CANUNDO, 0, 0L) ? MF_ENABLED : MF_GRAYED);

			//Enable Paste if text is in Clipboard
			EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);

			//Enable cut, copy and del if text is selected;
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&iSelBeg, (LPARAM)&iSelEnd);
			iEnable = iSelBeg != iSelEnd ? MF_ENABLED : MF_GRAYED;
			EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, iEnable);
			EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, iEnable);
			EnableMenuItem((HMENU)wParam, ID_EDIT_DELETE, iEnable);
			break;

		case 2:
			//search menu
			//Enable find, next, and replace if modelless, dialogs are not already active
			iEnable = hDlgModelless == NULL ? MF_ENABLED : MF_GRAYED;
			EnableMenuItem((HMENU)wParam, ID_SEARCH_FIND, iEnable);
			EnableMenuItem((HMENU)wParam, ID_SEARCH_FINDNEXT, iEnable);
			EnableMenuItem((HMENU)wParam, ID_SEARCH_REPLACE, iEnable);
			break;
		}
		return 0;

	case WM_COMMAND:
		//if message from edit control
		if (lParam && LOWORD(lParam) == EDITID) {
			switch (HIWORD(wParam)) {
			case EN_UPDATE:
				bNeedSave = TRUE;
				return 0;

			case EN_ERRSPACE:
			case EN_MAXTEXT:
				MessageBox(hwnd, TEXT("Edit control out of space"), szAppName, MB_ICONSTOP);
				return 0;
			}
			break;
		}

		//message from file menu
		switch (LOWORD(wParam)) {
		case ID_FILE_NEW:
			if (bNeedSave && IDCANCEL == AskAboutSave(hwnd, szTitleName)) {
				return 0;
			}

			SetWindowText(hwnd, TEXT("\0"));
			szFileName[0] = '\0';
			szTitleName[0] = '\0';
			DoCaption(hwnd, szTitleName);
			bNeedSave = FALSE;
			return 0;

		case ID_FILE_OPEN:
		{
			if (bNeedSave && IDCANCEL == AskAboutSave(hwnd, szTitleName)) {
				return 0;
			}

			if (PopFileOpenDlg(hwnd, szFileName, szTitleName)) {
				if (!PopFileRead(hwnd, szFileName)) {
					static TCHAR szTemp[] = TEXT("Could not read file %s");
					OkMessage(hwnd, szTemp, szTitleName);
					szFileName[0] = '\0';
					szTitleName[0] = '\0';
				}
			}
			DoCaption(hwnd, szTitleName);
			bNeedSave = FALSE;
		}
			return 0;

		case ID_FILE_SAVE:
		{
			if (szFileName[0]) {
				if (PopFileWrite(hwndEdit, szFileName)) {
					bNeedSave = FALSE;
					return 1;
				}
				else
				{
					static TCHAR szTemp[] = TEXT("Could not write file %s");
					OkMessage(hwnd, szTemp, szTitleName);
					return 0;
				}
			}
		}

		case ID_FILE_SAVEAS:
		{
			if (PopFileSaveDlg(hwnd, szFileName, szTitleName)) {
				DoCaption(hwnd, szTitleName);

				if (PopFileWrite(hwndEdit, szFileName)) {
					bNeedSave = FALSE;
					return 1;
				}
				else
				{
					static TCHAR szTemp[] = TEXT("Could not write file %s");
					OkMessage(hwnd, szTemp, szTitleName);
					return 0;
				}
			}
		}
			return 0;

		case ID_FILE_PRINT:
		{
			if (!PopPrintPrintFile(hInst, hwnd, hwndEdit, szTitleName)) {
				static TCHAR szTemp[] = TEXT("Could not print file %s");
				OkMessage(hwnd, szTemp, szTitleName);
			}
		}
			return 0;

		case ID_FILE_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;

		

		//Message from edit menu

		case ID_EDIT_UNDO:
			SendMessage(hwndEdit, WM_UNDO, 0, 0);
			return 0;

		case ID_EDIT_CUT:
			SendMessage(hwndEdit, WM_CUT, 0, 0);
			return 0;

		case ID_EDIT_COPY:
			SendMessage(hwndEdit, WM_COPY, 0, 0);
			return 0;

		case ID_EDIT_PASTE:
			SendMessage(hwndEdit, WM_PASTE, 0, 0);
			return 0;

		case ID_EDIT_DELETE:
			SendMessage(hwndEdit, WM_CLEAR, 0, 0);
			return 0;

		case ID_EDIT_SELECTALL:
			SendMessage(hwndEdit, EM_SETSEL, 0, -1);
			return 0;


		//message from search menu

		case ID_SEARCH_FIND:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)iOffset);
			hDlgModelless = PopFindFindDlg(hwnd);
			return 0;

		case ID_SEARCH_FINDNEXT:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)iOffset);
			if (PopFindValidFind()) {
				PopFindNextText(hwndEdit, &iOffset);
			}
			else
			{
				hDlgModelless = PopFindFindDlg(hwnd);
			}
			return 0;

		case ID_SEARCH_REPLACE:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)iOffset);
			hDlgModelless = PopFindReplaceDlg(hwnd);
			return 0;

		case ID_FORMAT_FONT:
			if (PopFontChooseFont(hwnd)) {
				PopFontSetFont(hwndEdit);
			}
			return 0;

		case ID_HELP_HELP:
		{
			static TCHAR szTemp[] = TEXT("HElp is not implemented!");
			OkMessage(hwnd, szTemp, szTitleName);
		}
			return 0;

		case ID_HELP_ABOUTPOPPAD:
			DialogBox(hInst, MAKEINTRESOURCE(ABOUT_DLG),hwnd, AboutDlgProc);
			return 0;

		}
		break;

	case WM_CLOSE:
		if (!bNeedSave || IDCANCEL != AskAboutSave(hwnd, szTitleName)) {
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_QUERYENDSESSION:
		if (!bNeedSave || IDCANCEL != AskAboutSave(hwnd, szTitleName)) {
			return 1;
		}
		return 0;


	case WM_DESTROY:
		PopFontDeinitilize();
		PostQuitMessage(0);
		return 0;

	default:
	{
		static TCHAR szTemp[] = TEXT("TExt not found");
		//process find replace message
		if (message == messageFindReplace) {
			pfr = (LPFINDREPLACE)lParam;

			if (pfr->Flags & FR_DIALOGTERM)
				hDlgModelless = NULL;

			if (pfr->Flags & FR_FINDNEXT)
				if (!PopFindFindText(hwndEdit, &iOffset, pfr))
					OkMessage(hwnd, szTemp, szTitleName);

			if (pfr->Flags & FR_REPLACEALL)
				while (PopFindReplaceText(hwndEdit, &iOffset, pfr));
			return 0;
		}
	}
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL CALLBACK AboutDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hdlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}