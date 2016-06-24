#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#define ENDTIME 30
#define MAXBUF 1024

void switchAction(int sig);
void timerElapsed(int sig);
void childTerminate(int sig);
void parentTerminate(int sig);

int action = 1;

int main(int argc, char** argv) {
	
	FILE* fp;
	char buf[MAXBUF], *filename;
	int row, timeInterval, timeTotal;
	pid_t pid;
	
	/* Check number of arguments */
	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments. Syntax: %s filename\n", argv[0]);
		return -1;
	}
	
	/* Set the filename */
	filename = argv[1];
	
	if ((pid = fork()) == -1) {
		/* Error */
		fprintf(stderr, "Error, fork() failed.\n");
		return -1;
	} else if (pid > 0) {
		/* The Parent */
		
		/* Instantiate the signal handlers */
		signal(SIGUSR1, switchAction);
		signal(SIGUSR2, parentTerminate);
		
		/* Open the file */
		if ((fp = fopen(filename, "r")) == NULL) {
			fprintf(stderr, "Error opening the file %s.\n", filename);
			return -1;
		}
		
		while(1) {
			row = 1;
			while(fgets(buf, MAXBUF, fp) != NULL) {
				if (action) {
					fprintf(stdout, "Row %d: %s\n", row, buf);
				}
				row++;
			}
			rewind(fp);
		}
		
	} else {
		/* The Child */
		
		timeTotal = 0;
		
		/* Seed the rand() function */
		srand(time(NULL));
		
		do {
			signal(SIGALRM, timerElapsed);
			timeInterval = (rand() % 10) + 1;
			timeTotal += timeInterval;
			alarm(timeInterval);
			pause();
		} while(timeTotal + timeInterval < ENDTIME);
		
		signal(SIGALRM, childTerminate);
		alarm(ENDTIME - timeTotal);
		pause();
	}
	
	return 0;
}

void switchAction(int sig) {
	action = !action;
	return;
}

void timerElapsed(int sig) {
	kill(getppid(), SIGUSR1);
	return;
}

void childTerminate(int sig) {
	kill(getppid(), SIGUSR2);
	raise(SIGTERM);
	return;
}

void parentTerminate(int sig) {
	raise(SIGTERM);
	return;
}
