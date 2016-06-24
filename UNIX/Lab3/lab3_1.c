#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "office.h"

void* office(void*); // Normal offices
void* specialOffice(void*); // Special office
void* student(void*); // Students

int main(int argc, char** argv) {

	int i;
	int* arg_th;
	pthread_t *students_th, *offices_th, special_office_th;
	void* ret;

	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments. Syntax: %s k\n", argv[0]);
		return -1;
	}
	
	/* Seed rand() */
	srand(time(NULL));
	
	/* Initialize k */
	k = atoi(argv[1]);

	/* Initialize and allocate num_students */
	num_students = *(Num_students*)malloc(sizeof(Num_students));
	pthread_mutex_init(&num_students.lock, NULL);
	pthread_mutex_lock(&num_students.lock);
	num_students.num = k;
	pthread_mutex_unlock(&num_students.lock);
	
	/* Initialize and allocate the buffers */
	normal_Q = B_init(k);
	special_Q = B_init(k);
	urgent_Q = (Buffer**)malloc(NUM_OFFICES*sizeof(Buffer*)); // for each office
	answer_Q = (Buffer**)malloc(k*sizeof(Buffer*)); // for each student
	
	/* Initialize and allocate Cond* cond.
	 * In order to implement the special office work
	 * similar to the normal ones, we use the cond variable
	 * at NUM_OFFICES position (see specialOffice() for details)
	 */
	cond = cond_init(NUM_OFFICES + 1);

	/* Allocate and create students */
	students_th = (pthread_t*)malloc(k*sizeof(pthread_t));
	for (i=0; i<k; i++) {
		arg_th = (int*)malloc(sizeof(int));
		*arg_th = i;
		if (pthread_create(&students_th[i], NULL, student, arg_th)) {
			fprintf(stderr, "Error creating student %d\n", i);
			return -1;
		}
	}
	
	/* Allocate and create offices */
	offices_th = (pthread_t*)malloc(NUM_OFFICES*sizeof(pthread_t));
	for (i=0; i<k; i++) {
		arg_th = (int*)malloc(sizeof(int));
		*arg_th = i;
		if (pthread_create(&offices_th[i], NULL, office, arg_th)) {
			fprintf(stderr, "Error creating office %d\n", i);
			return -1;
		}
	}
	
	/* Create special office */
	if (pthread_create(&special_office_th, NULL, specialOffice, NULL)) {
			fprintf(stderr, "Error creating special office\n");
			return -1;
		}

	pthread_exit(&ret);
}

/* Initialize and allocate the buffers */ 
Buffer* B_init(int dim) {
	Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
	buffer->buffer = (Info*)malloc(dim*sizeof(Info)); 
	pthread_mutex_init(&buffer->lock, NULL);
	buffer->notfull = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(buffer->notfull, NULL);
	buffer->notempty = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(buffer->notempty, NULL);
	buffer->in = 0;
	buffer->out = 0;
	buffer->count = 0;
	buffer->dim = dim;
	
	return buffer;
}

/* Initialize and allocate Cond* cond */
Cond* cond_init(int n) {
	Cond* cond = (Cond*)malloc(sizeof(Cond));
	pthread_mutex_init(&cond->lock, NULL);
	cond->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond->cond, NULL);
	cond->urgent = (int*)malloc(n*sizeof(int));
	cond->normal = 0;
	
	return cond;
}

/* A student */
void* student(void* arg_th) {

	int studentID = *(int*)arg_th;
	Info info;
	
	pthread_detach(pthread_self()); // make it not joinable
	
	/* Initialize answer queue and info */
	answer_Q[studentID] = B_init(2);
	info.id = studentID;
	info.office_no = NUM_OFFICES;
	info.urgent = 0;

	sleep((rand() % 9) + 1);
	
	/* Enter the normal queue */
	send(normal_Q, info);
	
	/* Wait to be served */
	info = receive(answer_Q[info.id]);
	fprintf(stdout, "student %d terminated after service at office %d\n", info.id, info.office_no);
	
	if (info.urgent == 1) {
		/* Enter the special queue to get additional information */
		fprintf(stdout, "student %d going from normal office to special office\n", info.id);
		send(special_Q, info);
		
		/* Wait to be served from special office */
		info = receive(answer_Q[info.id]);
		
		/* Enter the urgent queue */
		fprintf(stdout, "student %d going back to office %d\n", info.id, info.office_no);
		send(urgent_Q[info.office_no], info);
		
		/* Wait to be served from office with precedence */
		info = receive(answer_Q[info.id]);
		fprintf(stdout, "student %d completed at office %d\n", info.id, info.office_no);
	}
	
	fprintf(stdout, "student %d terminated after service at office %d\n", info.id, info.office_no);
	
	pthread_mutex_lock(&num_students.lock);
	num_students.num--;
	if (num_students.num == 0) {
		pthread_mutex_unlock(&num_students.lock);
		raise(SIGKILL);
	} else {
		pthread_mutex_unlock(&num_students.lock);
		pthread_exit(arg_th);
	}
	
	pthread_exit(arg_th);
}

/* A normal office */
void* office(void* arg_th) {

	int officeID = *(int*)arg_th;
	int urgent;
	Info info;
	
	pthread_detach(pthread_self()); // make it not joinable
	
	/* Initialize answer queue and info */
	urgent_Q[officeID] = B_init(k/3); // less space than normal queue
	
	/* Normal office service */
	while(1) {
		pthread_mutex_lock(&cond->lock);
		
		/* While there are no students to serve */
		while(cond->urgent[officeID]==0 && cond->normal==0) {
			pthread_cond_wait(cond->cond, &cond->lock);
		}
		
		if (cond->urgent[officeID] != 0) { // there are some students with priority
			/* Serve a student from urgent buffer */
			cond->urgent[officeID]--;
			pthread_mutex_unlock(&cond->lock);
			
			/* Receive info from student */
			info = receive(urgent_Q[officeID]);
			fprintf(stdout, "student %d received answer from office %d\n", info.id, officeID);
			
			sleep(1); // Service length
			
			info.urgent = 0;
			
			/* Send an aswer */
			send(answer_Q[info.id], info);
			
		} else { // no students with priority
			/* Serve a student from normal buffer */
			cond->normal--;
			pthread_mutex_unlock(&cond->lock);
			
			/* Receive info from student */
			info = receive(normal_Q);
			fprintf(stdout, "student %d received answer from office %d\n", info.id, officeID);
			
			sleep((rand() % 4) + 3); // Service length (between 3 and 6)
			
			/* Compute answer */
			info.office_no = officeID;
			urgent = rand() % 10;	// Urgent with 40% of probability
			if (urgent < 4) {
				info.urgent = 1; // Special service needed
			} else {
				info.urgent = 0; // No special service needed
			}
			
			/* Send an answer */
			send(answer_Q[info.id], info); 
		}
	}
	
	pthread_exit(arg_th);
}

/* The special office */
void* specialOffice(void* arg_th) {

	Info info;
	
	pthread_detach(pthread_self()); // make it not joinable
	
	while(1) {
		pthread_mutex_lock(&cond->lock);
		
		/* While there are no students to serve */
		while(cond->urgent[NUM_OFFICES]==0) {
			pthread_cond_wait(cond->cond, &cond->lock);
		}
		
		/* Serve a student */
		cond->urgent[NUM_OFFICES]--;
		pthread_mutex_unlock(&cond->lock);
		
		/* Receive info from student */
		info = receive(special_Q);
		fprintf(stdout, "student %d served by the special office\n", info.id);
		
		sleep((rand() % 4) + 3); // Service length
		
		/* Send an aswer */
		send(answer_Q[info.id], info);
	}
	
	pthread_exit(arg_th);
} 

/* Write data into the buffer */
void send(Buffer *buf, Info info) {

	pthread_mutex_lock(&buf->lock);
	
	/* Wait for some empty space in the buffer */
	while(buf->count == buf->dim) {
		pthread_cond_wait(buf->notfull, &buf->lock);
	}
	
	/* Write data */
	buf->buffer[buf->in] = info;
	buf->in++;
	if (buf->in == buf->dim) {
		buf->in = 0; // cyclic buffer
	}
	buf->count++;
	
	pthread_cond_broadcast(buf->notempty); // The buffer has some info to process, awakes waiting offices
	
	pthread_mutex_lock(&cond->lock);
	if (info.urgent == 1) {
		if (buf == urgent_Q[info.office_no]) {
			cond->urgent[info.office_no]++;
		} else { // (buf == special_Q)
			cond->urgent[NUM_OFFICES]++;
		}
	} else { // (info.urgent == 0) && (buf == normal_Q)
		cond->normal++;		
	}
	
	pthread_cond_broadcast(cond->cond);
	
	pthread_mutex_unlock(&cond->lock);
	pthread_mutex_unlock(&buf->lock);
}

/* Read data from the buffer */
Info receive(Buffer *buf) {
	
	Info info;

	pthread_mutex_lock(&buf->lock);
	
	/* Wait for some info to read in the buffer */
	while (buf->count == 0) {
		pthread_cond_wait(buf->notempty, &buf->lock);
	}
	
	/* Read data */
	info = buf->buffer[buf->out];
	buf->out++;
	if (buf->out == buf->dim) {
		buf->out = 0; // cyclic buffer
	}
	buf->count--;
	
	pthread_cond_broadcast(buf->notfull);
	pthread_mutex_unlock(&buf->lock);
	
	return info;
}
