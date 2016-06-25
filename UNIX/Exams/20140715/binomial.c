#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct Counter {
	pthread_mutex_t mutex;
	int cnt;
} Counter;

int n, k, numThreads, finished = 0;
int *numer, *denom;
float result = 1.0;
Counter countNumerFactors, countDenomFactors, countNumerBarrier, countDenomBarrier;

sem_t *barrier1, *barrier2;

void* thNumerator(void* arg);
void* thDenominator(void* arg);

int main(int argc, char** argv) {

	int i, *arg;
	pthread_t* th;

	/* Check arguments */
	if (argc != 4) {
		fprintf(stderr, "Wrong number of arguments. Syntax: %s n k numThreads\n", argv[0]);
		return -1;
	}
	
	/* Set n, k and numThreads */
	n = atoi(argv[1]);
	k = atoi(argv[2]);
	numThreads = atoi(argv[3]);
	
	/* Check n and k */
	if (((n == 0) && (k == 0)) || (n == k)) {
		fprintf(stdout, "The binomial coefficient is: 1\n");
		return 0;
	}
	
	/* Check number of threads */
	if (numThreads >= k/2) {
		numThreads = k/2;
	}
	
	/* Allocate and initialize the sempahores */
	barrier1 = (sem_t*)malloc(sizeof(sem_t));
	barrier2 = (sem_t*)malloc(sizeof(sem_t));
	sem_init(barrier1, 0, 0);
	sem_init(barrier2, 0, 0);
	
	/* Initialize counters */
	countNumerFactors.cnt = n-k+1;
	pthread_mutex_init(&countNumerFactors.mutex, NULL);
	countDenomFactors.cnt = 1;
	pthread_mutex_init(&countDenomFactors.mutex, NULL);
	countNumerBarrier.cnt = 0;
	pthread_mutex_init(&countNumerBarrier.mutex, NULL);
	countDenomBarrier.cnt = 0;
	pthread_mutex_init(&countDenomBarrier.mutex, NULL);
	
	/* Allocate and initialize numer and denom */
	numer = (int*)malloc(numThreads*sizeof(int));
	denom = (int*)malloc(numThreads*sizeof(int));
	for (i=0; i<numThreads; i++) {
		numer[i] = 1;
		denom[i] = 1;
	}
	
	/* Allocate threads */
	th = (pthread_t*)malloc(2*numThreads*sizeof(pthread_t));
	
	/* Create threads */
	for (i=0; i<numThreads; i++) {
		arg = (int*)malloc(sizeof(int));
		*arg = i;
		pthread_create(&th[i], NULL, (void*)thNumerator, arg);
		pthread_create(&th[i+numThreads], NULL, (void*)thDenominator, arg);
	}
	
	/* Wait for threads */
	for (i=0; i<numThreads*2; i++) {
		pthread_join(th[i], NULL);
	}
	
	fprintf(stdout, "The binomial coefficient is: %.2f\n", result);

	return 0;
}

void* thNumerator(void* arg) {
	int* argid = (int*) arg;
	int id, i;
	
	id = *argid;

	while (!finished) {
		
		/* Compute numer[id] */
		pthread_mutex_lock(&countNumerFactors.mutex);
		if (countNumerFactors.cnt <= n-1) {
			numer[id] = countNumerFactors.cnt * (countNumerFactors.cnt + 1);
			countNumerFactors.cnt += 2;
		} else if (countNumerFactors.cnt == n) {
			numer[id] = n;
			countNumerFactors.cnt++;
		} else {
			numer[id] = 1;
			countNumerFactors.cnt++;
		}
		pthread_mutex_unlock(&countNumerFactors.mutex);
		
		/* Increment the barrier counters, if threads completed compute the result,
		   check factors counters to know if this is the end. */
		pthread_mutex_lock(&countNumerBarrier.mutex);
		pthread_mutex_lock(&countDenomBarrier.mutex);
		countNumerBarrier.cnt++;
		if (countNumerBarrier.cnt == numThreads && countDenomBarrier.cnt == numThreads) {
			for (i=0; i<numThreads; i++) {
				result *= numer[i] / (float) denom[i];
				numer[i] = 1;
				denom[i] = 1;
			} 
			pthread_mutex_lock(&countNumerFactors.mutex);
			pthread_mutex_lock(&countDenomFactors.mutex);
			if (countNumerFactors.cnt > n && countDenomFactors.cnt > k) {
				finished = 1;
			}
			pthread_mutex_unlock(&countDenomFactors.mutex);
			pthread_mutex_unlock(&countNumerFactors.mutex);
			
			for (i=0; i<numThreads*2; i++) {
				sem_post(barrier1);
			}
		}
		pthread_mutex_unlock(&countDenomBarrier.mutex);
		pthread_mutex_unlock(&countNumerBarrier.mutex);
		
		sem_wait(barrier1);
		
		/* Reset the barrier counters to start again if there are still factors to multiply */
		pthread_mutex_lock(&countNumerBarrier.mutex);
		pthread_mutex_lock(&countDenomBarrier.mutex);
		countNumerBarrier.cnt--;
		if (countNumerBarrier.cnt == 0 && countDenomBarrier.cnt == 0) {
			for (i=0; i<numThreads*2; i++) {
				sem_post(barrier2);
			}
		}
		pthread_mutex_unlock(&countDenomBarrier.mutex);
		pthread_mutex_unlock(&countNumerBarrier.mutex);
		
		sem_wait(barrier2);
	}	

	pthread_exit(0);
}

void* thDenominator(void* arg) {

	int* argid = (int*) arg;
	int id, i;
	
	id = *argid;

	while (!finished) {
		
		/* Compute denom[id] */
		pthread_mutex_lock(&countDenomFactors.mutex);
		if (countDenomFactors.cnt <= k-1) {
			denom[id] = countDenomFactors.cnt * (countDenomFactors.cnt + 1);
			countDenomFactors.cnt += 2;
		} else if (countDenomFactors.cnt == k) {
			denom[id] = k;
			countDenomFactors.cnt++;
		} else {
			denom[id] = 1;
			countDenomFactors.cnt++;
		}
		pthread_mutex_unlock(&countDenomFactors.mutex);
		
		/* Increment the barrier counters, if threads completed compute the result,
		   check factors counters to know if this is the end. */
		pthread_mutex_lock(&countNumerBarrier.mutex);
		pthread_mutex_lock(&countDenomBarrier.mutex);
		countDenomBarrier.cnt++;
		if (countDenomBarrier.cnt == numThreads && countDenomBarrier.cnt == numThreads) {
			for (i=0; i<numThreads; i++) {
				result *= numer[i] / (float) denom[i];
				numer[i] = 1;
				denom[i] = 1;
			} 
			pthread_mutex_lock(&countNumerFactors.mutex);
			pthread_mutex_lock(&countDenomFactors.mutex);
			if (countNumerFactors.cnt > n && countDenomFactors.cnt > k) {
				finished = 1;
			}
			pthread_mutex_unlock(&countDenomFactors.mutex);
			pthread_mutex_unlock(&countNumerFactors.mutex);
			
			for (i=0; i<numThreads*2; i++) {
				sem_post(barrier1);
			}
		}
		pthread_mutex_unlock(&countDenomBarrier.mutex);
		pthread_mutex_unlock(&countNumerBarrier.mutex);
		
		sem_wait(barrier1);
		
		/* Reset the barrier counters to start again if there are still factors to multiply */
		pthread_mutex_lock(&countNumerBarrier.mutex);
		pthread_mutex_lock(&countDenomBarrier.mutex);
		countDenomBarrier.cnt--;
		if (countNumerBarrier.cnt == 0 && countDenomBarrier.cnt == 0) {
			for (i=0; i<numThreads*2; i++) {
				sem_post(barrier2);
			}
		}
		pthread_mutex_unlock(&countDenomBarrier.mutex);
		pthread_mutex_unlock(&countNumerBarrier.mutex);
		
		sem_wait(barrier2);
	}	

	pthread_exit(0);
}
