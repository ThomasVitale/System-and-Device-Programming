#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

int finished=0, n, totalServices=0;
sem_t *mutex, *print, *process;
void* clientTh(void* arg);

static void manager(int sig) {
	if (++finished == 2) {
		fprintf(stdout, "Total number of served requests: %d \n", totalServices);
		exit(0);
	}
}

int main(int argc, char** argv) {

	int i[2] = {1, 2};
	pthread_t th1, th2;
	
	/* Signal handler */
	if (signal(SIGUSR1, manager) == SIG_ERR) {
		fprintf(stderr, "Error SIGUSR1\n");
		return -1;
	}
	
	/* Allocate sempahores */
	mutex = malloc(sizeof(sem_t*));
	print = malloc(sizeof(sem_t*));
	process = malloc(sizeof(sem_t*));
	
	/* Initialize semaphores */
	sem_init(mutex, 0, 1);
	sem_init(print, 0, 0);
	sem_init(process, 0, 0);
	
	/* Create the two threads */
	if (pthread_create(&th1, NULL, (void*)clientTh, (void*)&i[0]) != 0) {
		fprintf(stderr, "Error creating thread 1 \n");
		return -1;
	}
	
	if (pthread_create(&th2, NULL, (void*)clientTh, (void*)&i[1]) != 0) {
		fprintf(stderr, "Error creating thread 2 \n");
		return -1;
	}
	
	/* Serve clients */
	while(1) {
		sem_wait(process);
		n *= 2;
		totalServices++;
		sem_post(print);
	}

	return 0;
}

void* clientTh(void* arg) {

	int* id = (int*)arg;
	FILE* fp;
	
	/* Open the file */
	if (*id == 1) {
		if ((fp = fopen("fv1.b", "r")) == NULL) {
			fprintf(stderr, "Error opening file fv1.b");
			return (void*)-1;
		}
	} else {
		if ((fp = fopen("fv2.b", "r")) == NULL) {
			fprintf(stderr, "Error opening file fv2.b");
			return (void*)-1;
		}
	}
	
	/* Read and print */
	while(1) {
	
		sem_wait(mutex);
	
		if (fread(&n, sizeof(int), 1, fp) == 0) {
			sem_post(mutex);
			break;
		}
		
		sem_post(process);
		sem_wait(print);
		
		fprintf(stdout, "Tid: %d \t n=%d \n", (int)pthread_self(), n);
		
		sem_post(mutex);
		
	}
	
	fclose(fp);
	
	raise(SIGUSR1);

	pthread_exit(NULL);
}
