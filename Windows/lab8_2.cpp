#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

typedef struct Student {
	INT id;
	INT number;
	TCHAR name[30];
	TCHAR surname[30];
	INT mark;
} Student;

int _tmain(int argc, LPTSTR argv[]) {

	FILE* fIn;
	HANDLE hOut;
	Student s;
	DWORD nIn, nOut;
	TCHAR c;

	/* Check number of arguments */
	if (argc != 3) {
		_ftprintf(stderr, _T("Wrong number of arguments. Usage: %s fileIN.txt fileOUT.txt\n"), argv[0]);
		return -1;
	}

	/* Open input file for reading */
	if ((fIn = _tfopen(argv[1], _T("r"))) == NULL) {
		_ftprintf(stderr, _T("Error opening the input file: %d\n"), GetLastError());
		return -2;
	}

	/* Open output file for writing */
	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error opening the output file: %d\n"), GetLastError());
		return -3;
	}

	/* Write output file converting from ASCII to binary */
	while (_ftscanf(fIn, _T("%i %i %s %s %i"), &s.id, &s.number, s.name, s.surname, &s.mark) > 0) {
		if (!WriteFile(hOut, &s, sizeof(Student), &nOut, NULL)) {
			_ftprintf(stderr, _T("Error writing the output file: %d\n"), GetLastError());
			return -4;
		}
	}

	/* Close both files */
	fclose(fIn);
	CloseHandle(hOut);

	/* Open output file for reading and checking the result */
	hOut = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error opening the output file: %d\n"), GetLastError());
		return -5;
	}

	/* Read the output file and print out on the console for checking */
	while (ReadFile(hOut, &s, sizeof(Student), &nIn, NULL) && (nIn > 0)) {
		_ftprintf(stdout, _T("%i %i %s %s %i\n"), s.id, s.number, s.name, s.surname, s.mark);
	}

	_ftprintf(stdout, _T("\nPress 'Enter' to exit.\n"));
	_ftscanf(stdin, _T("%c"), &c);

	CloseHandle(hOut);

	return 0;
}