#include <Windows.h>
#include <commdlg.h>

static OPENFILENAME ofn;

void PopFileInitilize(HWND hwnd) {
	static TCHAR szFilter[] = TEXT("Text Files (*.TXT)\0*.txt\0") \
		TEXT("ASCII Files (*.ASC)\0*.asc\0") \
		TEXT("All Files (*.*)\0*.*\0\0");

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}


BOOL PopFileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName) {
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	return GetOpenFileName(&ofn);
}


BOOL PopFileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName) {
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);

}


BOOL PopFileRead(HWND hwndEdit, PTSTR pstrFileName) {
	BYTE bySwap;
	DWORD dwBytesRead;
	HANDLE hFile;
	int i, iFileLEngth, iUnitTest;
	PBYTE pBuffer, pText, pConv;

	//open the file
	if (INVALID_HANDLE_VALUE == (hFile = CreateFile(pstrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))) {
		return FALSE;
	}

	//get the file size in bytes and allocate memory for read
	//add extra two bytes for zero termination

	iFileLEngth = GetFileSize(hFile, NULL);
	pBuffer = (PBYTE)malloc(iFileLEngth + 2);

	ReadFile(hFile, pBuffer, iFileLEngth, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLEngth] = '\0';
	pBuffer[iFileLEngth + 1] = '\0';

	//see if the text is unicode
	iUnitTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;

	if (IsTextUnicode(pBuffer, iFileLEngth, &iUnitTest)) {
		pText = pBuffer + 2;
		iFileLEngth -= 2;
		if (iUnitTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE) {
			for (i = 0; i < iFileLEngth / 2; i++) {
				bySwap = ((BYTE*)pText)[2 * i];
				((BYTE*)pText)[2 * i] = ((BYTE*)pText)[2 * i + 1];
				((BYTE*)pText)[2 * i + 1] = bySwap;
			}
		}
		pConv = (PBYTE)malloc(iFileLEngth + 2);

#ifndef UNICODE
		WideCharToMultiByte(CP_ACP, 0, (PWSTR)pText, -1, pConv, iFileLEngth + 2, NULL, NULL);

#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);

#endif
	}
	else {
		pText = pBuffer;
		pConv = (PBYTE)malloc(iFileLEngth + 2);

#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (LPCCH)pText, -1, (PTSTR)pConv, iFileLEngth + 1);

#else
		lstrcpy((PTSTR)pConv, (PTSRT)pText);

#endif
	}
	SetWindowText(hwndEdit, (PTSTR)pConv);
	free(pBuffer);
	free(pConv);
	return TRUE;

}


BOOL PopFileWrite(HWND hwndEdit, PTSTR pstrFileName) {
	DWORD dwBytesWritten;
	HANDLE hFile;
	int iLength;
	PTSTR pstrBuffer;
	WORD wByteOrderMark = 0xFEFF;

	//open the file or create if necessary
	if (INVALID_HANDLE_VALUE == (hFile = CreateFile(pstrFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL))) {
		return FALSE;
	}

	//get the number of character in the edit control and allocate memory for them
	iLength = GetWindowTextLength(hwndEdit);
	pstrBuffer = (PTSTR)malloc((iLength + 1) * sizeof(TCHAR));

	if (!pstrBuffer) {
		CloseHandle(hFile);
		return FALSE;
	}

#ifdef UNICODE
	WriteFile(hFile, &wByteOrderMark, 2, &dwBytesWritten, NULL);
#endif

	//get the edit buffer and write it to the file
	GetWindowText(hwndEdit, pstrBuffer, iLength + 1);
	WriteFile(hFile, pstrBuffer, iLength * sizeof(TCHAR), &dwBytesWritten, NULL);
	
	if ((iLength * sizeof(TCHAR)) != (int)dwBytesWritten) {
		CloseHandle(hFile);
		free(pstrBuffer);
		return FALSE;
	}

	CloseHandle(hFile);
	free(pstrBuffer);
	return TRUE;
}
