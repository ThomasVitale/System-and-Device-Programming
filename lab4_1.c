#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {

	FILE* fb_out;
	int i, n, x;

	/* Check number of arguments */
	if (argc != 3) {
		fprintf(stderr, "Syntax: %s n fileOutput\n", argv[0]);
		return -1;
	}
	
	n = atoi(argv[1]); // total number to be generated
	
	/* Open binary file for writing */
	if ((fb_out = fopen(argv[2], "wb")) == NULL) {
		fprintf(stderr, "Error opening the output file.\n");
		return -2;
	} 
	
	srand(time(NULL)); // seed rand() function
	
	/* Generate n random integers */
	for (i=0; i<n; i++) {
		x = rand();
		fwrite(&x, sizeof(int), 1, fb_out);
		fprintf(stdout, "%d ", x);
	}
	fprintf(stdout, "\n");
	
	fclose(fb_out);
	
	return 0;
}
