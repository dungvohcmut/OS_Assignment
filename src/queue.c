#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
    if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    /* TODO: put a new process to queue [q] */
    if (q->size < MAX_QUEUE_SIZE) {
	    q->proc[q->size] = proc;
	    q->size++;
	}
	else {
		printf("Error: queue is full, cannot enqueue process\n");
		exit(EXIT_FAILURE);
	}
}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: take a process from queue [q] following round robin - FCFS */
    if (empty(q)) {
		printf("Error: queue is empty, cannot dequeue process\n");
		exit(EXIT_FAILURE);
	}
	struct pcb_t * proc = q->proc[0];
	for (int i = 0; i < q->size - 1; i++) {
		q->proc[i] = q->proc[i + 1];
	}
	q->size--;
	return proc;
}

