#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
	int cnt;
	pthread_mutex_t mutex;
} Counter;

int k;
float result;
float *v, *v1, *v2;
float **mat;
Counter C;

void* operation (void* arg) {
	int* argT = arg;
	int i = *argT;
	int j;
	
	pthread_detach(pthread_self());
	
	/* Perform product and store in v[i] */
	v[i] = 0;
	for (j=0; j<k; j++) {
		v[i] += mat[i][j]*v2[i];
	}
	
	/* Update counter and check if this is the last thread */
	pthread_mutex_lock(&C.mutex);
	
	C.cnt--;
	
	if (C.cnt == 0) {
		/* Second operation: result = (v1T*v) */
		result = 0;
		for (i=0; i<k; i++) {
			result += v1[i]*v[i];
		}
	
		/* Print the result */
		fprintf(stdout, "Result = %f\n", result);
	}
	
	pthread_mutex_unlock(&C.mutex);
	
	return (void*)0;
}

int main (int argc, char** argv) {

	int i, j;
	int* arg;
	pthread_t th;

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
	
	/* Set the initial value of the counter */
	pthread_mutex_lock(&C.mutex);
	C.cnt = k;
	pthread_mutex_unlock(&C.mutex);
	
	/* Each thread i performs the product of the i-th row vector of mat and v2 */
	for (i=0; i<k; i++) {
	  arg = (int*) malloc(sizeof(int));
		*arg = i;
		pthread_create(&th, NULL, operation, arg);
	}

	pthread_exit(0);
}
