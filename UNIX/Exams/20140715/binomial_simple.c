#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

int n, k;
int numer, denom;
float result = 1;

sem_t *s1, *s2;

void* thNumerator(void* arg);
void* thDenominator(void* arg);

int main(int argc, char** argv) {

	pthread_t thN, thD;

	/* Check arguments */
	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments. Syntax: %s n k\n", argv[0]);
		return -1;
	}
	
	/* Set n and k */
	n = atoi(argv[1]);
	k = atoi(argv[2]);
	
	/* Allocate and initialize the sempahores */
	s1 = (sem_t*)malloc(sizeof(sem_t));
	s2 = (sem_t*)malloc(sizeof(sem_t));
	sem_init(s1, 0, 0);
	sem_init(s2, 0, 0);
	
	/* Create threads */
	pthread_create(&thN, NULL, (void*)thNumerator, NULL);
	pthread_create(&thD, NULL, (void*)thDenominator, NULL);
	
	/* Wait for threads */
	pthread_join(thN, NULL);
	pthread_join(thD, NULL);
	
	fprintf(stdout, "The binomial coefficient is: %.2f\n", result);

	return 0;
}

void* thNumerator(void* arg) {

	int i;
	
	i = n;
	while (i > (n-k+1)) {
		numer = i*(i-1);
		sem_wait(s1);
		result *= (float)numer/denom;
		sem_post(s2);
		i -= 2;
	}
	
	if (i == (n-k+1)) {
		numer = i;
	} else {
		numer = 0;
	}
	
	sem_wait(s1);
	
	if (numer != 0 && denom != 0) {
		result *= (float)numer/denom;
	} else if (numer != 0 && denom == 0) {
		result *= numer;
	} else if (numer == 0 && denom != 0) {
		result *= (float)1/denom;
	}

	pthread_exit(0);
}

void* thDenominator(void* arg) {

	int i;
	
	i = 1;
	while(i < k) {
		denom = i*(i+1);
		sem_post(s1);
		sem_wait(s2);
		i += 2;
	}
	
	if (i == k) {
		denom = i;
	} else {
		denom = 0;
	}
	
	sem_post(s1);

	pthread_exit(0);
}
