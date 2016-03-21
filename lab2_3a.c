#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int main (int argc, char** argv) {

	int k, i, j;
	float result;
	float *v, *v1, *v2;
	float **mat;

	if (argc != 2) {
		fprintf(stderr, "Syntax: %s k", argv[0]);
		return -1;
	}
	
	k = atoi(argv[1]);
	
	/* Memory allocation */
	
	v = (float*) malloc(k*sizeof(float));
	v1 = (float*) malloc(k*sizeof(float));	
	v2 = (float*) malloc(k*sizeof(float));
	mat = (float**) malloc(k*sizeof(float*));
	
	for (i=0; i<k; i++) {
		mat[i] = (float*) malloc(k*sizeof(float));
	}
	
	/* Arrays and matrix filling */
	
	srand(time(NULL)); // seeding rand() 
	
	fprintf(stdout, "v1:\n");
	for (i=0; i<k; i++) {
		v1[i] = ((float) (rand() % 100) / 100) - 0.5;
		fprintf(stdout, "%f ", v1[i]);
	}
	fprintf(stdout, "\n");
	
	fprintf(stdout, "v2:\n");
	for (i=0; i<k; i++) {
		v2[i] = ((float) (rand() % 100) / 100) - 0.5;
		fprintf(stdout, "%f ", v2[i]);
	}
	fprintf(stdout, "\n");
	
	fprintf(stdout, "matr:\n");
	for (i=0; i<k; i++) {
		for (j=0; j<k; j++) {
			mat[i][j] = ((float) (rand() % 100) / 100) - 0.5;
			fprintf(stdout, "%f ", mat[i][j]);
		}
		fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n");
	
	/* (v1T*mat*v2) is divided into two easier operations */
	/* a) v = (mat*v2) */
	
	for (i=0; i<k; i++) {
		v[i] = 0;
		for (j=0; j<k; j++) {
			v[i] += mat[i][j]*v2[i];
		}
	}
	
	/* b) result = (v1T*v) */
	
	result = 0;
	for (i=0; i<k; i++) {
		result += v1[i]*v[i];
	}
	
	/* Print the result */
	fprintf(stdout, "Result = %2f\n", result);

	return 0;
}
