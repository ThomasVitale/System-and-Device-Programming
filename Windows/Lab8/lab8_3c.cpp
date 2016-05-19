#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define L 30

typedef struct Student {
	INT id;
	INT number;
	TCHAR name[L];
	TCHAR surname[L];
	INT mark;
} Student;

int _tmain(int argc, LPTSTR argv[]) {

	HANDLE hIn, hOut;
	DWORD nIn, nOut, n;
	BOOL finish = FALSE;
	TCHAR input[L];
	TCHAR cmd;
	LARGE_INTEGER filePos, fileReserved;
	OVERLAPPED ov = { 0, 0, 0, 0, NULL };
	Student s;

	/* Check number of arguments */
	if (argc != 2) {
		_ftprintf(stderr, _T("Wrong number of arguments. Usage: %s fileIn\n"), argv[0]);
		return -1;
	}

	while (!finish) {
		_ftprintf(stdout, _T("Enter a command:\n"));
		_fgetts(input, L, stdin);

		cmd = input[0];

		/* Case E */
		if (cmd == 'E') {
			finish = TRUE;
		}

		/* Case R */
		if (cmd == 'R') {
			_stscanf(input, _T("%*c %i"), &n);

			hIn = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hIn == INVALID_HANDLE_VALUE) {
				_ftprintf(stderr, _T("Error opening the file for reading: %i\n"), GetLastError());
				return -2;
			}

			filePos.QuadPart = (n - 1) * sizeof(Student);
			ov.Offset = filePos.LowPart;
			ov.OffsetHigh = filePos.HighPart;

			fileReserved.QuadPart = sizeof(Student);

			/* Lock the record to be read */
			if (!LockFileEx(hIn, 0, 0, fileReserved.LowPart, fileReserved.HighPart, &ov)) {
				_ftprintf(stderr, _T("Error locking the file: %i\n"), GetLastError());
				return -4;
			}

			/* Read the record */
			if (ReadFile(hIn, &s, sizeof(Student), &nIn, &ov) && (nIn > 0)) {
				_ftprintf(stdout, _T("%i %i %s %s %i\n"), s.id, s.number, s.name, s.surname, s.mark);
			}
			else {
				_ftprintf(stderr, _T("Error reading the file: %i\n"), GetLastError());
				return -5;
			}

			/* Unlock the record to be read */
			if (!UnlockFileEx(hIn, 0, fileReserved.LowPart, fileReserved.HighPart, &ov)) {
				_ftprintf(stderr, _T("Error unlocking the file: %i\n"), GetLastError());
				return -6;
			}

			CloseHandle(hIn);

		}

		/* Case W */
		if (cmd == 'W') {
			_stscanf(input, _T("%*c %i"), &n);

			hOut = CreateFile(argv[1], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hOut == INVALID_HANDLE_VALUE) {
				_ftprintf(stderr, _T("Error opening the file for writing: %i\n"), GetLastError());
				return -7;
			}

			_ftprintf(stdout, _T("Enter a new record:\n"));
			_ftscanf(stdin, _T("%i %i %s %s %i"), &s.id, &s.number, s.name, s.surname, &s.mark);

			filePos.QuadPart = (n - 1) * sizeof(Student);
			ov.Offset = filePos.LowPart;
			ov.OffsetHigh = filePos.HighPart;

			fileReserved.QuadPart = sizeof(Student);

			/* Lock the record to be written */
			if (!LockFileEx(hOut, LOCKFILE_EXCLUSIVE_LOCK, 0, fileReserved.LowPart, fileReserved.HighPart, &ov)) {
				_ftprintf(stderr, _T("Error locking the file: %i\n"), GetLastError());
				return -8;
			}

			/* Write the record */
			if (!WriteFile(hOut, &s, sizeof(Student), &nOut, &ov) || (nOut != sizeof(Student))) {
				_ftprintf(stderr, _T("Error writing the file: %i\n"), GetLastError());
				return -9;
			}

			/* Unlock the record to be written */
			if (!UnlockFileEx(hOut, 0, fileReserved.LowPart, fileReserved.HighPart, &ov)) {
				_ftprintf(stderr, _T("Error unlocking the file: %i\n"), GetLastError());
				return -10;
			}

			CloseHandle(hOut);
		}
	}

	return 0;
}