// System and Device Programming
// Lab 1, excercise 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int randomNumber(int min, int max);
void sortVector(int* v, int n);
void printVector(int* v, int n);

int main(int argc, char** argv) {
	
	int n1, n2;
	int *v1, *v2;
	int i;
	FILE *fv1, *fv2, *fv1b, *fv2b;
	
	srand(time(NULL)); // Seeding the rand() function

	// Takes from the command line two integer numbers n1, n2

	if (argc != 3) {
		fprintf(stderr, "Error! The format is: %s int1 int2 \n", argv[0]);
		return -1;
	}

	n1 = atoi(argv[1]);
	n2 = atoi(argv[2]);

	// Allocates two vectors v1 and v2, of dimensions n1 and n2, respectively

	v1 = (int*) malloc(n1*sizeof(int));
	v2 = (int*) malloc(n2*sizeof(int));
	
	// Fills v1 with n1 random numbers between 10-100

	for (i=0; i<n1; i++) {
		v1[i] = randomNumber(10, 100);
	}
	fprintf(stdout, "%s", "Vector v1: ");
	printVector(v1, n1);
	
	// Fills v2 with n2 random numbers between 20-100

	for (i=0; i<n2; i++) {
		v2[i] = randomNumber(20, 100);
	}
	fprintf(stdout, "%s", "Vector v2: ");
	printVector(v2, n2);
	
	// Sort v1 and v2 (increasing values)
	
	sortVector(v1, n1);
	sortVector(v2, n2);
	
	fprintf(stdout, "%s", "Vector v1 (sorted): ");
	printVector(v1, n1);
	fprintf(stdout, "%s", "Vector v2 (sorted): ");
	printVector(v2, n2);

	// Save the content of vectors v1 and v2 in two text files fv1.txt and fv2.txt, respectively

	if ((fv1 = fopen("fv1.txt", "w+")) == NULL) {
		fprintf(stderr, "%s", "Error opening fv1.txt\n");
		return -2;
	}

	if ((fv2 = fopen("fv2.txt", "w+")) == NULL) {
		fprintf(stderr, "%s", "Error opening fv2.txt\n");
		return -3;
	}

	for (i=0; i<n1; i++) {
		fprintf(fv1, "%d ", v1[i]);
	}
	fprintf(stdout, "%s", "File fv1.txt scritto con successo \n");
	
	for (i=0; i<n2; i++) {
		fprintf(fv2, "%d ", v2[i]);
	}
	fprintf(stdout, "%s", "File fv2.txt scritto con successo \n");
	
	fclose(fv1);
	fclose(fv2);

	// Save the content of vectors v1 and v2 in two binary files fv1.b and fv2.b, respectively
	
	if ((fv1b = fopen("fv1.b", "w+")) == NULL) {
		fprintf(stderr, "%s", "Error opening fv1.b\n");
		return -4;
	}

	if ((fv2b = fopen("fv2.b", "w+")) == NULL) {
		fprintf(stderr, "%s", "Error opening fv2.b\n");
		return -5;
	}
	
	if (fwrite(v1, sizeof(int), n1, fv1b) != n1) {
		fprintf(stderr, "%s", "Error writing fv1.b");
		return -6;
	}
	fprintf(stdout, "%s", "File fv1.b scritto con successo \n");
	
	if (fwrite(v2, sizeof(int), n2, fv2b) != n2) {
		fprintf(stderr, "%s", "Error writing fv2.b");
		return -7;
	}
	fprintf(stdout, "%s", "File fv2.b scritto con successo \n");
	
	fclose(fv1b);
	fclose(fv2b);
	
	return 0;
}

int randomNumber(int min, int max) {

	int result;

	result = (rand() % (max - min + 1)) + min;

	return result;

}

void sortVector(int* v, int n) {
	int i, j, min, tmp;

	for (i=0; i<n-1; i++) {
		min = i;
		for (j=i+1; j<n; j++) {
			if (v[j] < v[min]) {
				min = j;
			}
		}
		tmp = v[i];
		v[i] = v[min];
		v[min] = tmp;
	}

	return;
}

void printVector(int* v, int n) {
	int i;

	for (i=0; i<n; i++) {
		fprintf(stdout, "%d ", v[i]);
	}

	fprintf(stdout, "\n");
}
