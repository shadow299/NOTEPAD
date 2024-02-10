#include <Windows.h>

HDC GetPrinterDC(void) {
	DWORD dwNeeded, dwReturned;
	HDC hdc;
	PRINTER_INFO_4* pinfo4 = NULL;

	// Get required buffer size
	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &dwNeeded, &dwReturned);
	if (dwNeeded == 0) {
		return NULL; // No printers found
	}

	// Allocate memory for printer info
	pinfo4 = (PRINTER_INFO_4*)malloc(dwNeeded);
	if (pinfo4 == NULL) {
		return NULL; // Memory allocation failed
	}

	// Enumerate printers and get printer info
	if (!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (PBYTE)pinfo4, dwNeeded, &dwNeeded, &dwReturned)) {
		free(pinfo4);
		return NULL; // Enumeration failed
	}

	// Check if any printers found
	if (dwReturned == 0) {
		free(pinfo4);
		return NULL; // No printers found
	}

	// Create printer DC
	hdc = CreateDC(NULL, pinfo4[3].pPrinterName, NULL, NULL);
	free(pinfo4); // Free allocated memory
	return hdc;
}