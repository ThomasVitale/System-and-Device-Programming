#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

int generateFile(int*, int, FILE*);

int _tmain(int argc, LPTSTR argv[]) {

	int a[] = {5, 23, 45, 67, 9, 23 };
	int b[] = {12, 90, 65, 34, 24, 12, 0, 89, 29, 54, 12, 78, 3};
	int c[] = {3, 9, 5, 1};

	FILE *fA, *fB, *fC;

	fA = fopen("file1.txt", "w+");
	fB = fopen("file2.txt", "w+");
	fC = fopen("file3.txt", "w+");

	generateFile(a, 6, fA);
	generateFile(b, 13, fB);
	generateFile(c, 4, fC);

	return 0;
}

int generateFile(int* v, int n, FILE* fIn) {

	if (fwrite(v, n * sizeof(int), n, fIn) < 0) {
		_ftprintf(stdout, _T("Error."));
	}

	return 0;
}