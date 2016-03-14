// System and Device Programming
// Lab 1, excercise 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <pthread.h>

// Write a C program that generate a child. The parent opens a file
// given as argument in the command line, then loops forever,
// 		1) reading each line,
// 		2) printing the line number and the line content
// 		3) rewinding the file.
//
// - The child process sends a SIGUSR1 signal to the parent at random intervals 
//   between 1 and 10 seconds. The first received signal must force the parent 
//   to skip the print statement.
// - The next received SIGUSR1 signal must force the parent to restart printing.
// - This behaviour is repeated for all the received SIGUSR1 signals. After 60 seconds, 
//   the child must send a SIGUSR2 signal to the parent, and then terminates. Receiving 
//   this signal, also the parent will terminate.

void* lastSignal();
int c=2; // global variable for computing if SIGUSR1 received has ben sent in even or odd position
int action=1; // global variable for switching the parent process between printing and pausing

static void manager(int sig) {
	
	if (sig == SIGUSR1) {
		if (c++ % 2) {
			action = 0;
			return;
		} else {
			action = 1;
			return;
		}
	} else {
		raise(SIGKILL);
		return;
	}
}

int main(int argc, char** argv) {
	
	char* filename;
	FILE* fd;
	pid_t pid;
	pthread_t tid;
	char buffer[1024];
	int row = 1;
	
	if (signal(SIGUSR1, manager) == SIG_ERR) {
		fprintf(stderr, "Error SIGUSR1\n");
		return -1;
	}
	
	if (signal(SIGUSR2, manager) == SIG_ERR) {
		fprintf(stderr, "Error SIGUSR2\n");
		return -2;
	}
	
	if (argc != 2) {
		fprintf(stderr, "Error! The format is: %s filename \n", argv[0]);
		return -3;
	}
	
	filename = (char*) malloc(strlen(argv[1])*sizeof(char));
	strcpy(filename, argv[1]);
	
	pid = fork();
	
	if (pid == -1) { // error
		fprintf(stderr, "Error with the fork operation!\n");
		return -4;
	} else if (pid > 0) { // the father
		
		if ((fd = fopen(filename, "r")) == NULL) {
			fprintf(stderr, "Error opening the file %s \n", filename);
			return -5;
		}

		while (1) {
			if (action) {
				if (fgets(buffer, 1024, fd) == NULL) {
					row = 1;
					rewind(fd);
				} else {
					fprintf(stdout, "%d \t", row);
					fputs(buffer, stdout);
					row++;
				}
			} else {
				pause();
			}
		}
	
	} else { // the child
	
		if (pthread_create(&tid, NULL, lastSignal, NULL) != 0) {
			fprintf(stderr, "Error generating the thread lastSignal");
			return -6;
		}
	
		srand(time(NULL));
		
		while (1) {
			sleep(rand() % 10 + 1);
			kill(getppid(), SIGUSR1);
		}
	}
	
	return 0;
}

void* lastSignal() {

	sleep(60);
	kill(getppid(), SIGUSR2);
	raise(SIGKILL);
	
	return (void*)0;
}
