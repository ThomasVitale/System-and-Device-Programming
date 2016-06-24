#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
  int* left;
  int* right;
} Region;

typedef struct {
	pthread_mutex_t lock;
	sem_t sa;
	sem_t sb;
	int counter;
} Barrier;

static void* threadSorter (void*);
void swap(int*, int*);

int* paddr;
int len, n, nthreads;
Barrier barrier;

int main (int argc, char** argv) {

	int fd;
	int i;
	struct stat stat_buf;
	Region** regions;
  pthread_t* threads;
  
  /* Check the number of arguments */
  if (argc != 2) {
    fprintf (stderr, "Wrong number of arguments. Syntax: %s filename\n", argv[0]);
    return -1;
  }
  
  /* Open the file passed as argument */
  if ((fd = open (argv[1], O_RDWR)) == -1) {
    fprintf(stderr, "Error opening the file.\n");
    return -1;
  }
	
	/* Compute the length of the binary file */
  if (fstat (fd, &stat_buf) == -1) {
    fprintf(stderr, "Error using fstat() function.\n");
    return -1;
  }
  len = stat_buf.st_size; // number of bytes written in the file file
  n = len / sizeof(int); // number of integer written in the file
	
	/* Map the file in memory */
  paddr = mmap ((void*) 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)0);
  
  if ((int*)paddr == (int*)-1) {
    fprintf(stderr, "Error using mmap() function.\n");
    return -1;
  }

	/* Close the input file */
  close (fd);
  
  /* Compute the number of threads to be generated according to the formula
   * nthreads = ceiling(log10(number_of_integers_in_file).
   */
  nthreads = ceil(log10(n));
  fprintf(stdout, "The number of threads to be generated: %d\n", nthreads);
  
  /* Compute the length of each region, except the last one that will have
   * also all the remaining numbers
   */
  len = n/nthreads;
  
  /* Initialise the Barrier structure */
  pthread_mutex_init(&barrier.lock, NULL);
  pthread_mutex_lock(&barrier.lock);
  sem_init(&barrier.sa, 0, 0);
  sem_init(&barrier.sb, 0, 0);
  barrier.counter = nthreads;
  pthread_mutex_unlock(&barrier.lock);
  
  /* Allocation of regions and threads */
  regions = (Region**) malloc(nthreads*sizeof(Region*));
  threads = (pthread_t*) malloc(nthreads*sizeof(pthread_t));
  
  /* Creation of regions and threads */
  for (i=0; i<nthreads; i++) {
  
  	/* Fill in regions */
  	regions[i] = (Region*) malloc(sizeof(Region));
  	regions[i]->left = paddr + (i*len);
  	if (i == nthreads-1) {
  		// The last thread
  		regions[i]->right = paddr + (n-1);
  	} else {
  		// Not the last thread
  		regions[i]->right = paddr + ((i+1)*len - 1);
  	}
  	
  	/* Create threads */
  	if (pthread_create(&threads[i], NULL, threadSorter, regions[i]) != 0) {
  		fprintf(stderr, "Error creating thread %d\n", i);
  		return -1;
  	}
  	
  }
  
  pthread_exit((void*)0);
  
  return 0;
}

static void* threadSorter (void *arg) {
	Region* region = (Region*) arg;
	int *x, *y;
	int *left = region->left;
	int *right = region->right;
  int i, swapped;
  
  while (1) {
  	x = left;
  	
  	while (x <= right) {
  		y = x;
  		while (y > left && *(y-1) > *y) {
  			swap(y-1, y);
  			y--;
  		}
  		x++;
  	}
  	
  	/* Barrier processing */
		pthread_mutex_lock(&barrier.lock);
		barrier.counter--;
		if (barrier.counter == 0) {
			// The last thread reaching the barrier swaps the borders
			swapped = 0;
			for (i=1; i<nthreads; i++) {
				if (*(paddr + (i*len) - 1) > *(paddr + (i*len))) {
					// Swap borders
					swap(paddr + (i*len) - 1, paddr + (i*len));
					swapped = 1;
				}
			}
			if (swapped) {
				// Since there was a swap, sort again the region
				for (i=0; i<nthreads; i++) {
					sem_post(&barrier.sa);
				}
			} else {
				x = paddr;
				i = 0;
				while (x<paddr+n) {
					fprintf(stdout, "%i : %i\n", i, *x);
					x++;
					i++;
				}
				exit(0);
			}
		}
		pthread_mutex_unlock(&barrier.lock);
		
		sem_wait(&barrier.sa);
		
		pthread_mutex_lock(&barrier.lock);
		barrier.counter++;
		if (barrier.counter == nthreads) {
			for (i=0; i<nthreads; i++) {
				sem_post(&barrier.sb);
			}
		}
		pthread_mutex_unlock(&barrier.lock);
		sem_wait(&barrier.sb);
	}
	
	return arg;
}

/* Swap two integers */
void swap(int* x, int* y) {
	int tmp = *x;
	*x = *y;
	*y = tmp;
}
