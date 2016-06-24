#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void fillArray(int* v, int n, int min, int max);
void sortArray(int* v, int n);
void writeArray(int* v, int n, char* filename);
void writeArrayBin(int* v, int n, char* filename);

int main(int argc, char** argv) {
	
	int n1, n2;
	int *v1, *v2;

	/* Check arguments */
	if (argc != 3) {
		fprintf(stderr, "Wrong number of parameters. Syntax: %s n1 n2\n", argv[0]);
		return -1;
	}
	
	/* Get the two arguments */
	n1 = atoi(argv[1]);
	n2 = atoi(argv[2]);
	
	/* Allocate the two arrays */
	v1 = (int*)malloc(n1*sizeof(int));
	v2 = (int*)malloc(n2*sizeof(int));
	
	/* Fill the two arrays */
	fillArray(v1, n1, 10, 100);
	fillArray(v2, n2, 20, 100);
	
	/* Sort the two arrays */
	sortArray(v1, n1);
	sortArray(v2, n2);
	
	/* Write v1 and v2 in two text files */
	writeArray(v1, n1, "fv1.txt");
	writeArray(v2, n2, "fv2.txt");
	
	/* Write v1 and v2 in two binary files */
	writeArrayBin(v1, n1, "fv1.b");
	writeArrayBin(v2, n2, "fv2.b");
	
	return 0;
}

/* Fill an array with random numbers within a specified range */
void fillArray(int* v, int n, int min, int max) {

	int i, range;
	
	/* Compute the range */
	range = max - min;
	
	/* Seed the rand() function */
	srand(time(NULL));
	
	for (i=0; i<n; i++) {
		v[i] = (rand() % (range+1)) + min;
	}

	return;
}

/* Sort an array usign the Bubble Sort algorithm */
void sortArray(int* v, int n) {
	
	int i, j, tmp;
	
	/* Bubble sort */
	for (i=0; i<n-1; i++) {
		for (j=i; j<n-1-i; j++) {
			if (v[j] > v[j+1]) {
				tmp = v[j];
				v[j] = v[j+1];
				v[j+1] = tmp;
			}
		}
	}
	
	return;
}

/* Save an array into a text file */
void writeArray(int* v, int n, char* filename) {

	FILE* fp;
	int i;
	
	if ((fp = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "Error opening the file %s for writing.\n", filename);
		return;
	}
	
	for (i=0; i<n; i++) {
		fprintf(fp, "%d ", v[i]);
	}
	fprintf(fp, "\n");
	
	fclose(fp);
	
	return;

}

/* Save an array into a binary file */
void writeArrayBin(int* v, int n, char* filename) {

	FILE* fp;
	
	if ((fp = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "Error opening the file %s for writing.\n", filename);
		return;
	}
	
	if (fwrite(v, sizeof(int), n, fp) != n) {
		fprintf(stderr, "Error writing the file %s.\n", filename);
		return;
	}
	
	fclose(fp);
	
	return;

}
