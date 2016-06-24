#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Vector {
	pthread_mutex_t mutex;
	int* vet;
} Vector;

Vector V; // Data used by threads in mutual exclusion
int t; // Threshold, under this value directly sort instead creating other threads

void merge(int *vet, int left, int middle, int right);
void* threadedMergeSort(int* args);

int main(int argc, char** argv) {

  int i, n;
  int args[2];
	
	/* Check arguments */
  if (argc != 3) {
    fprintf (stderr, "Wrong number of arguments. Syntax: %s dimension threshold\n", argv[0]);
    return -1;
  }
  
  /* Set the array dimension and threshold */
  n = atoi(argv[1]);
  t = atoi(argv[2]);
	
	/* Allocate the array */
  V.vet = (int*)malloc(n*sizeof(int));
  
  /* Initialize mutex */
  pthread_mutex_init(&V.mutex, NULL);
  
  /* Seed the rand() function */
  srand(n);
  
  /* Fill the array */
  fprintf(stdout, "Array before sorting: ");
  pthread_mutex_lock(&V.mutex);
  for(i = 0; i < n; i++) {
    V.vet[i] = rand() % 100;
		fprintf(stdout, "%d ", V.vet[i]);
  }
	pthread_mutex_unlock(&V.mutex);
	fprintf(stdout, "\n");
	
	/* Call the threaded version of mergesort */ 
	args[0] = 0; // left
	args[1] = n-1; // right
	
	/* Sort the array */  
  threadedMergeSort(args);
	
	/* Print the array */
  fprintf(stdout, "Array after sorting: ");
  pthread_mutex_lock(&V.mutex);
  for(i = 0; i < n; i++) {
		fprintf(stdout, "%d ", V.vet[i]);
	}
	pthread_mutex_unlock(&V.mutex);
	fprintf(stdout, "\n");
	
  return 0;
}

void* threadedMergeSort(int* args) {

	int* arg = args;
	int left, middle, right;
	int argsA[2], argsB[2];
	pthread_t threadA, threadB;
	void* retValue;
	
	left = arg[0];
	right = arg[1];
	
	if (left < right) {

		middle = left + (right - left)/2;  
		
		if ((right - left) >= t) {
			/* Create threadA */
			argsA[0] = left;
			argsA[1] = middle;
			pthread_create(&threadA, NULL, (void*)threadedMergeSort, (void*)argsA);
		
			/* Create threadB */
			argsB[0] = middle + 1;
			argsB[1] = right;
			pthread_create(&threadB, NULL, (void*)threadedMergeSort, (void*)argsB);
		
			/* Wait for threads */
			pthread_join(threadA, &retValue);
			pthread_join(threadB, &retValue);
		} else {
			/* Below the threshold, just sort it */
			argsA[0] = left;
			argsA[1] = middle;
			argsB[0] = middle + 1;
			argsB[1] = right;
			threadedMergeSort(argsA);
			threadedMergeSort(argsB);
		}
		
		/* Merge results in mutual exclusion */
		pthread_mutex_lock(&V.mutex);
		merge(V.vet, left, middle, right);
		pthread_mutex_unlock(&V.mutex);
	
	}
  
  return (void*)0;
}

void merge(int *vet, int left, int middle, int right) {
    
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    int L[n1], R[n2];  // temp arrays
    
    for (i = 0; i <= n1; i++) {  // make a copy
    	L[i] = vet[left + i];
    }
    for (j = 0; j <= n2; j++) {
      R[j] = vet[middle + 1 + j];
    }
        
    // Merge the temp arrays in vet[l..r]
    i = 0; 
    j = 0; 
    k = left; // Initial index of merged subarray
    while (i < n1 && j < n2)  {
      if (L[i] <= R[j]) {
        vet[k++] = L[i++];
      } else {
        vet[k++] = R[j++];
      }
    }
    // Copy the remaining elements of L[]
    while (i < n1) {
      vet[k++] = L[i++];
    }
    // Copy the remaining elements of R[]
    while (j < n2) {
      vet[k++] = R[j++];
    }
    
    return;
}
