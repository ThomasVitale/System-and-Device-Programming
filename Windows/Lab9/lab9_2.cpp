#define UNICODE
#define _UNICODE 

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

DWORD WINAPI OrderingThread(LPVOID);
VOID merge(DWORD, int*, DWORD, int*, int*);
int printFile(LPTSTR);
int printArray(int*, int);

typedef struct {
	LPTSTR fileName;
	DWORD recordNumber;
	DWORD index;
	int* records;
} arguments_t;

arguments_t* args;

int _tmain(int argc, LPTSTR argv[]) {

	DWORD fileNumber, numberTerminatedThreads = 0, firstTerminatedThread;
	DWORD numberJustTerminatedThread, indexJustTerminatedThread;
	DWORD waitValue, i, outN = 0;
	HANDLE* thHandles;
	LPDWORD thIDs;
	int* outRecords;
	int* mapThreadArgs;

	HANDLE hOut;
	DWORD nOut;

	TCHAR c;

	/* Check number of arguments and allocate fileNumber */
	if (argc < 3) {
		_ftprintf(stdout, _T("Wrong number of arguments.\n"));
		return -1;
	}

	fileNumber = argc - 2;

	/* Allocate handles, ids and args arrays */
	thHandles = (HANDLE*)malloc(fileNumber * sizeof(HANDLE));
	thIDs = (LPDWORD)malloc(fileNumber * sizeof(DWORD));
	args = (arguments_t*)malloc(fileNumber * sizeof(arguments_t));
	mapThreadArgs = (int*)malloc(fileNumber * sizeof(int));

	/* Create threads */
	for (i = 0; i < fileNumber; i++) {
		args[i].fileName = argv[i + 1];
		args[i].index = i + 1;
		thHandles[i] = CreateThread(NULL, 0, OrderingThread, &args[i], 0, &thIDs[i]);
		if (thHandles[i] == INVALID_HANDLE_VALUE) {
			_ftprintf(stdout, _T("Error creating thread %i.\n"), i + 1);
			return -2;
		}
		mapThreadArgs[i] = i;
	}

	/* Wait for the ordering threads and merge as soon as two have finished */
	while (numberTerminatedThreads != fileNumber) {

		// Wait for one ordering thread and check if failed
		waitValue = WaitForMultipleObjects(fileNumber - numberTerminatedThreads, thHandles, FALSE, INFINITE);
		if (waitValue == WAIT_FAILED) {
			_ftprintf(stdout, _T("Error waiting for oredring threads.\n"));
			return -3;
		}

		// Find the current terminated thread
		if ((waitValue >= WAIT_OBJECT_0) && (waitValue <= WAIT_OBJECT_0 + fileNumber)) {

			numberTerminatedThreads++;

			// The index for accessing handles array
			numberJustTerminatedThread = waitValue - WAIT_OBJECT_0;

			// The index for accessing args structure
			i = 0;
			while (mapThreadArgs[i] != numberJustTerminatedThread) {
				i++;
			}
			indexJustTerminatedThread = i;

			// Close just terminated ordering thread handles
			if (!CloseHandle(thHandles[numberJustTerminatedThread])) {
				_ftprintf(stdout, _T("Error closing handle %i.\n"), numberJustTerminatedThread);
				return -4;
			}

			// Update the handles array deleting the just terminated thread
			for (i = numberJustTerminatedThread; i < fileNumber - numberTerminatedThreads; i++) {
				thHandles[i] = thHandles[i + 1];
			}

			// Update the map array disabling the just terminated thread and considering the above edit to handles array
			for (i = indexJustTerminatedThread; i < fileNumber; i++) {
				mapThreadArgs[i]--;
			}
			mapThreadArgs[indexJustTerminatedThread] = -1;

			// Merge results into outRecords array
			if (numberTerminatedThreads >= 2) {
				// args[firstTerminatedThread] is used to add up new arrays for merging
				outN = args[firstTerminatedThread].recordNumber + args[indexJustTerminatedThread].recordNumber;
				outRecords = (int*)malloc(outN * sizeof(int));
				merge(args[firstTerminatedThread].recordNumber, args[firstTerminatedThread].records, args[indexJustTerminatedThread].recordNumber, args[indexJustTerminatedThread].records, outRecords);
				args[firstTerminatedThread].recordNumber = outN;
				free(args[firstTerminatedThread].records);
				args[firstTerminatedThread].records = outRecords;
			}
			// Save the first terminated thread
			else {
				firstTerminatedThread = numberJustTerminatedThread;
			}
		}

	}

	/* Print final array */
	printArray(args[firstTerminatedThread].records, args[firstTerminatedThread].recordNumber);

	/* Open the output file */
	hOut = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stdout, _T("Error opening output file %s.\n"), argv[argc - 1]);
		return -4;
	}

	/* Write the number of total records */
	if (!WriteFile(hOut, &args[firstTerminatedThread].recordNumber, sizeof(int), &nOut, NULL) || nOut != sizeof(int)) {
		_ftprintf(stdout, _T("Error writing output file.\n"));
		return -5;
	}

	i = 0;
	while (WriteFile(hOut, &args[firstTerminatedThread].records[i], sizeof(int), &nOut, NULL) && (nOut == sizeof(int)) && (i < outN - 1)) {
		i++;
	}

	/* Close output file handle */
	if (!CloseHandle(hOut)) {
		_ftprintf(stdout, _T("Error closing output file.\n"));
		return -7;
	}

	/* Read and print the output file for checking it */
	printFile(argv[argc - 1]);

	/* Ask for exiting */
	_ftprintf(stdout, _T("\nPress 'Enter' to exit.\n"));
	_ftscanf(stdin, _T("%c"), &c);

	return 0;
}

/* Each thread order a file */
DWORD WINAPI OrderingThread(LPVOID argTH) {

	arguments_t* arg = (arguments_t*)argTH;
	HANDLE hIn;
	DWORD nIn;
	int i, j, n, tmp;

	/* Open input file */
	hIn = CreateFile(arg->fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stdout, _T("Error opening file %s.\n"), arg->fileName);
		ExitThread(arg->index);
	}

	/* Read number of items */
	if (!ReadFile(hIn, &n, sizeof(int), &nIn, NULL) || (nIn != sizeof(int))) {
		_ftprintf(stdout, _T("Error reading file the first time %s.\n"), arg->fileName);
		ExitThread(arg->index);
	}

	/* Store recordNumber and allocate records array */
	arg->recordNumber = n;
	arg->records = (int*)malloc(n * sizeof(int));

	/* Read items and store them into records array */
	if (!ReadFile(hIn, arg->records, n * sizeof(int), &nIn, NULL) && (nIn != n * sizeof(int))) {
		_ftprintf(stdout, _T("Error reading file %s.\n"), arg->fileName);
		ExitThread(arg->index);
	}

	/* Close input file handle */
	if (!CloseHandle(hIn)) {
		_ftprintf(stdout, _T("Error closing file %s.\n"), arg->fileName);
		ExitThread(arg->index);
	}

	/* Sort records array using the Bubble Sort algorithm */
	for (i = 0; i < n - 1; i++) {
		for (j = 0; j < n - 1 - i; j++) {
			if (arg->records[j] > arg->records[j + 1]) {
				tmp = arg->records[j];
				arg->records[j] = arg->records[j + 1];
				arg->records[j + 1] = tmp;
			}
		}
	}

	ExitThread(0);
}

/* Merge and sort two arrays */
VOID merge(DWORD nA, int* A, DWORD nB, int* B, int* Z) {
	DWORD i, j, k;

	i = j = k = 0;

	while (i < nA && j < nB) {
		if (A[i] < B[j]) {
			Z[k++] = A[i++];
		}
		else {
			Z[k++] = B[j++];
		}
	}

	while (i < nA) {
		Z[k++] = A[i++];
	}

	while (j < nB) {
		Z[k++] = B[j++];
	}

	return;
}

int printFile(LPTSTR outputFile) {
	HANDLE hIn;
	DWORD nIn;
	int item;

	/* Open the output file */
	hIn = CreateFile(outputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stdout, _T("Error opening output file.\n"));
		return -8;
	}

	/* Read and print final records */
	while (ReadFile(hIn, &item, sizeof(int), &nIn, NULL) && (nIn == sizeof(int))) {
		_ftprintf(stdout, _T("%d "), item);
	}

	_ftprintf(stdout, _T("\n"));

	return 0;
}

int printArray(int* v, int n) {
	int i;

	for (i = 0; i < n; i++) {
		_ftprintf(stdout, _T("%i "), v[i]);
	}
	_ftprintf(stdout, _T("\n"));

	return 0;
}