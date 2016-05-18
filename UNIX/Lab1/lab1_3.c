// System and Device Programming
// Lab 1, excercise 3

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

// Implement a concurrent program in C language, using Pthreads,
// which creates two client threads, then it acts as a server.
// A client thread loops reading the next number from the binary file
// (fv1.b and fv2.b, respectively), and storing it in a global
// variable. Then, it signals to the server that the variable is ready,
// and it waits on a semaphore a signal from the server indicating that
// the number has been processed (simply multiplied by 2), finally, it
// prints the result and its identifier.
// The server loops waiting the signals of the clients, doing the
// multiplication, storing the results on the same global variable, and
// signalling to the client that the string can be printed.
// The main thread waits the end on the threads, and prints the total
// number of served requests.

void *client1();
void *client2();

int n;
int join=0;
int totalNumbers=0;

sem_t *process;
sem_t *next;
sem_t *mutex;
sem_t *print;

static void manager(int sig) {
	fprintf(stdout, "Total number of served requests: %d \n", totalNumbers);
	exit(0);
}

int main() {

	pthread_t tid1, tid2;
	
	// This thread is signaled when the two clients thread finished.
	// Here a flag is used in order to know when to stop the while loop.
	// Another way to do this is creating another thread that executes
	// the loop, while the main thread waits using pthread_join() instructions.
	
	if (signal(SIGUSR1, manager) == SIG_ERR) {
		fprintf(stderr, "Error SIGUSR1\n");
		return -1;
	}
	
	process = malloc(sizeof(sem_t*));
	next = malloc(sizeof(sem_t*));
	mutex = malloc(sizeof(sem_t*));
	print = malloc(sizeof(sem_t*));
	
	sem_init(process, 0, 0);
	sem_init(next, 0, 1);
	sem_init(mutex, 0, 1);
	sem_init(print, 0, 0);
	
	if (pthread_create(&tid1, NULL, client1, NULL) != 0) {
		fprintf(stderr, "Error creating thread 1 \n");
		return -2;
	}
	
	if (pthread_create(&tid2, NULL, client2, NULL) != 0) {
		fprintf(stderr, "Error creating thread 2 \n");
		return -3;
	}
	 
	while(1) {
		sem_wait(process);
		n *= 2;
		totalNumbers++;
		sem_post(print);
		sem_post(next);
	}
	
	fprintf(stdout, "Total number of served requests: %d \n", totalNumbers);

	return 0;
}

void *client1() {

	FILE *fv1;
	
	if ((fv1 = fopen("fv1.b", "r")) == NULL) {
		fprintf(stderr, "Error opening file fv1.b");
		return (void*)-4;
	}

	while(1) {
	
		sem_wait(next);
		sem_wait(mutex);
	
		if (fread(&n, sizeof(int), 1, fv1) == 0) {
			sem_post(next);
			sem_post(mutex);
			break;
		}
		
		sem_post(process);
		sem_wait(print);
		
		fprintf(stdout, "Tid: %d \t n=%d \n", (int)pthread_self(), n);
		
		sem_post(mutex);
		
	}
	
	if (++join == 2) {
		raise(SIGUSR1);
	}
	
	pthread_exit(NULL);
}

void *client2() {

	FILE *fv2;
	
	if ((fv2 = fopen("fv2.b", "r")) == NULL) {
		fprintf(stderr, "Error opening file fv2.b");
		return (void*)-5;
	}

	while(1) {
	
		sem_wait(next);
		sem_wait(mutex);
	
		if (fread(&n, sizeof(int), 1, fv2) == 0) {
			sem_post(next);
			sem_post(mutex);
			break;
		}
		
		sem_post(process);
		sem_wait(print);
		
		fprintf(stdout, "Tid: %d \t n=%d \n", (int)pthread_self(), n);
		
		sem_post(mutex);
	}
	
	if (++join == 2) {
		raise(SIGUSR1);
	}
	
	pthread_exit(NULL);
}
