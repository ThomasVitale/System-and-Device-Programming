#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXLEN 1023
#define N 4

typedef struct Item {
	char* dir;
	struct Item* next;
} Item;

int fd, numDir[N];
Item* head;
pthread_mutex_t mep, mec, mef;
sem_t *full;
struct timespec ts;

void* exploreDirectory(void* arg);
Item* get(void);
void put(char* item);

int main(int argc, char** argv) {

	char *directory, *filename;
	int *arg, i; 
	pthread_t th;
	struct stat statBuf;

	/* Check arguments */
	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments. Syntax: %s directory filename\n", argv[0]);
		return -1;
	}
	
	/* Set directory and filename */
	directory = argv[1];
	filename = argv[2];
	
	/* Check item existance */
	if (stat(directory, &statBuf) == -1) {
		fprintf(stderr, "ERROR. stat() failed.\n");
		return -1;
	}
	
	/* Check if it's an actual directory */
	if (!S_ISDIR(statBuf.st_mode)) {
		fprintf(stderr, "ERROR. %s is not a directory.\n", directory);
		return -1;
	}
	
	/* Open the file for writing */
	if ((fd = open(filename, O_CREAT | O_RDWR, 0777)) == -1) {
		fprintf(stderr, "ERROR. open() failed.\n");
		return -1;
	}
	
	/* Initialize the mutexes */
	pthread_mutex_init(&mep, NULL);
	pthread_mutex_init(&mec, NULL);
	pthread_mutex_init(&mef, NULL);
	
	/* Allocate and initialize the semaphore */
	full = (sem_t*)malloc(sizeof(sem_t));
	sem_init(full, 0, 0);
	
	/* Put the first item in the list */
	put(directory);
	
	/* Create threads */
	for (i=0; i<N; i++) {
		arg = (int*)malloc(sizeof(int));
		*arg = i;
		pthread_create(&th, NULL, (void*)exploreDirectory, (void*)arg);
	}
	
	pthread_exit((void*)0);
}

/* Get an Item from the list */
Item* get(void) {
	Item* item;
	
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 1;
	if (sem_timedwait(full, &ts) == -1) {
		return (Item*)NULL;
	}
	
	pthread_mutex_lock(&mec);
	item = head;
	head = head->next;
	pthread_mutex_unlock(&mec);
	
	return item;
}

/* Put an Item into the list */
void put(char* itemData) {
	Item* item;
	
	pthread_mutex_lock(&mep);
	item = (Item*)malloc(sizeof(Item));
	item->dir = strdup(itemData);
	item->next = head;
	head = item;
	pthread_mutex_unlock(&mep);
	sem_post(full);
	
	return;
}

void* exploreDirectory(void* arg) {

	int* id = (int*)arg;
	char dirName[MAXLEN + 1], *pathName, row[MAXLEN + 6];
	DIR *dp;
	Item* item;
	struct dirent *dirEntry;
	struct stat statBuf;
	
	pthread_detach(pthread_self());
	
	while(1) {
		
		/* Get a directory to explore from the list */
		item = get();
		if (item == NULL) {
			// No directories to esplore
			fprintf(stdout, "Thread %d explored %d directories.\n", *id, numDir[*id]);
			pthread_exit((void*)0);
		}
		
		/* Open the directory */
		pathName = item->dir;
		if ((dp = opendir(pathName)) == NULL) {
			fprintf(stderr, "ERROR. opendir() failed.\n");
			exit(1);
		}
		numDir[*id]++; // how many directories processed by this thread
		
		/* Read the directory itself */
		while((dirEntry = readdir(dp)) != NULL) {
			sprintf(dirName, "%s/%s", pathName, dirEntry->d_name);
			
			// Get info about the current entry
			if (lstat(dirName, &statBuf) == -1) {
				fprintf(stderr, "ERROR. lstat() failed.\n");
				exit(1);
			}
			
			// Check the entry type 
			switch(statBuf.st_mode & S_IFMT) {
				case S_IFDIR: // Directory
					// If it's not the current or the parent directory
					if (strcmp(dirEntry->d_name, ".") && strcmp(dirEntry->d_name, "..")) {
						fprintf(stdout, "%s is a directory explored by thread %d.\n", dirName, *id);
						// Add to the list
						put(dirName);
						sleep(1);
					}
					break;
				case S_IFREG: // File
					fprintf(stdout, "%s is a file processed by thread %d.\n", dirName, *id);
					pthread_mutex_lock(&mef);
					// Write the entry on the file
					sprintf(row, "%d %s\n", *id, dirName);
					write(fd, row, strlen(row));
					write(fd, "\n", 1);
					pthread_mutex_unlock(&mef);
					break;
				default:
					break;
			}
		}
		
		closedir(dp);
		free(item);
		
	}
	
	pthread_exit((void*)0);
}















