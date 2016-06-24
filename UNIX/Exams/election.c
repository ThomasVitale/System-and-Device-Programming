#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define N 10

typedef struct Leader {
	pthread_mutex_t mutex;
	int best_rank;
	int cnt;
	long int tid;
} Leader;

Leader* leader;
sem_t* barrier;

int randomRank(int max);
void* thRoutine(void* thArg);

int main(int argc, char** argv) {
	
	int *arg, i, j, k, ranks[N];
	pthread_t th;
	
	/* Seed the rand() function */
	srand(time(NULL));
	
	/* Allocate and initialize the leader */
	leader = (Leader*)malloc(sizeof(Leader));
	pthread_mutex_init(&leader->mutex, NULL);
	leader->best_rank = 0;
	leader->cnt = 0;
	
	/* Allocate and initialize the barrier */
	barrier = (sem_t*)malloc(sizeof(sem_t));
	sem_init(barrier, 0, 0);
	
	/* Create the threads */
	for (i=0; i<N; i++) {
		ranks[i] = i;
	}
	k = N;
	for (i=0; i<N; i++) {
		arg = (int*)malloc(sizeof(int));
		j = randomRank(k);
		*arg = ranks[i] + 1;
		ranks[j] = ranks[--k];
		pthread_create(&th, NULL, (void*)thRoutine, (void*)arg);
	}
	
	pthread_exit(0);
}

int randomRank(int max) {
	int r;
	
	r = rand() % max;
	
	return (int)r;
}

void* thRoutine(void* thArg) {

	int* rank = (int*)thArg;
	int i;
	long int id;
	
	id = pthread_self();
	pthread_detach(pthread_self());

	pthread_mutex_lock(&leader->mutex);
	if (leader->best_rank < *rank) {
		leader->best_rank = *rank;
		leader->tid = id;
	}
	leader->cnt++;
	if (leader->cnt == N) {
		for(i=0; i<N; i++) {
			sem_post(barrier);
		}
	}
	pthread_mutex_unlock(&leader->mutex);
	
	sem_wait(barrier);
	
	fprintf(stdout, "Thread ID: %ld\tThread Rank: %d\tLeader ID: %ld\tLeader Rank: %d\n", id, *rank, leader->tid, leader->best_rank);

	pthread_exit((void*)0);
}
