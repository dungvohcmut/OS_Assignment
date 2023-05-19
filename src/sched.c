#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
#define MAX_PRIO 140
static struct queue_t mlq_ready_queue[MAX_PRIO];
static unsigned int prio_state = 0;
static unsigned int mlq_time_slot[MAX_PRIO];
#elif MLFQ_SCHED
#define MAX_PRIO 3
static struct queue_t mlfq_ready_queue[MAX_PRIO];
static struct queue_t mlfq_contain_queue;
static unsigned int prio_state = 0;
static unsigned int mlfq_time_slot[MAX_PRIO];
#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#elif MLFQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlfq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
	int i;

	for (i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].size = 0;
		mlq_time_slot[i] = MAX_PRIO - i;
	}
#elif MLFQ_SCHED
	int i;
	for (i = 0; i < MAX_PRIO; i++)
	{
		mlfq_ready_queue[i].size = 0;
		mlfq_time_slot[i] = MAX_PRIO - i;
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t *get_mlq_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (!empty(&mlq_ready_queue[prio_state]) && mlq_time_slot[prio_state] != 0)
	{
		proc = dequeue(&mlq_ready_queue[prio_state]);
		mlq_time_slot[prio_state]--;
	}
	else
	{
		mlq_time_slot[prio_state] = MAX_PRIO - prio_state;
		int i = (prio_state + 1) % MAX_PRIO;
		while (i != prio_state)
		{
			if (!empty(&mlq_ready_queue[i]))
			{
				proc = dequeue(&mlq_ready_queue[i]);
				mlq_time_slot[i]--;
				prio_state = i;
				break;
			}
			else
			{
				i = (i + 1) % MAX_PRIO;
			}
		}
	}
	
	if (proc == NULL)
	{
		if (!empty(&mlq_ready_queue[prio_state]))
		{
			proc = dequeue(&mlq_ready_queue[prio_state]);
			mlq_time_slot[prio_state]--;
		}
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	return add_mlq_proc(proc);
}

#elif MLFQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLFQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t *get_mlfq_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (!empty(&mlfq_ready_queue[prio_state]) && mlfq_time_slot[prio_state] != 0)
	{
		proc = dequeue(&mlfq_ready_queue[prio_state]);
		mlfq_time_slot[prio_state]--;
	}
	else
	{
		mlfq_time_slot[prio_state] = MAX_PRIO - prio_state;
		int i = (prio_state + 1) % MAX_PRIO;
		if (i == 0) {
			struct pcb_t *process = NULL;
			while (!empty(&mlfq_contain_queue)) {
				process = dequeue(&mlfq_contain_queue);
				enqueue(&mlfq_ready_queue[0], process);
			}
		}
		while (i != prio_state)
		{
			if (!empty(&mlfq_ready_queue[i]))
			{
				proc = dequeue(&mlfq_ready_queue[i]);
				mlfq_time_slot[i]--;
				prio_state = i;
				break;
			}
			else
			{
				i = (i + 1) % MAX_PRIO;
			}
		}
	}

	if (proc == NULL)
	{
		if (!empty(&mlfq_ready_queue[prio_state]))
		{
			proc = dequeue(&mlfq_ready_queue[prio_state]);
			mlfq_time_slot[prio_state]--;
		}
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_mlfq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlfq_ready_queue[0], proc);
	pthread_mutex_unlock(&queue_lock);
}


void add_mlfq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlfq_ready_queue[0], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlfq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlfq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	return add_mlfq_proc(proc);
}
#else
struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (!empty(&ready_queue))
	{
		proc = dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif
