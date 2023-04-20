#include <stdlib.h>
#include <stdio.h>
#include <valgrind/valgrind.h>

#include "queue.h"
#include "thread.h"

void add_tail(struct queue *q, struct node *n)
{
	// printf("ADDED to QUEUE %p\n", n);
	// printf("QUEUE TAIL :%p\n", q->tail);
	if (q->tail)
		q->tail->next = n;
	else
		q->head = n;

	q->tail = n;
	// printf("QUEUE HEAD TAIL :%p %p\n", q->head, q->tail);
}

void add_head(struct queue *q, struct node *n)
{
	n->next = q->head;
	q->head = n;
	if (!q->tail)
		q->tail = n;
}

struct node *get_head(struct queue *q)
{
	return q->head;
}

struct node *get_tail(struct queue *q)
{
	return q->tail;
}

struct node *pop_head(struct queue *q)
{
	struct node *n = q->head;

	if (q->head)
		q->head = q->head->next;

	if (!q->head)
		q->tail = NULL;

	n->next = NULL;
	return n;
}

void remove_node(struct queue *q, struct node *n)
{
	if (!n)
		return;

	if (q->head == n)
	{
		q->head = q->head->next;
		if (q->tail == n)
		{ // If this is true, the queue is empty
			q->tail = NULL;
		}
		return;
	}

	struct node *tmp = q->head;
	while (tmp && tmp->next != n)
	{
		tmp = tmp->next;
	}
	if (!tmp)
	{
		// printf("[%s:%s] Error: Node not found in queue", __FILE__, __func__);
		return;
	}
	tmp->next = tmp->next->next;
	if (tmp->next == NULL)
	{
		q->tail = tmp;
	}
}

int queue_empty(struct queue *q)
{
	return q->head == NULL;
}

// Appending q2 to the end of q1
void append_queue(struct queue *q1, struct queue *q2)
{
	if(queue_empty(q1)) {
		q1->head = q2->head;
		q1->tail = q2->tail;
		return;
	}

	q1->tail->next = q2->head;
	q1->tail = q2->tail;
}

struct node *new_node(ucontext_t *context)
{
	struct node *n = malloc(sizeof(struct node));
	n->uc = context;
	n->dirty = 0;
	n->retval = NULL;
	n->next = NULL;
	n->valgrind_stackid = 0;
	n->waiters_queue = NULL;
	return n;
}

void free_node(struct node *n)
{
	if (!n)
		return;

	free_node(n->next);
	if (!n->dirty && n->uc != main_context)
	{
		VALGRIND_STACK_DEREGISTER(n->valgrind_stackid);
		free(n->uc->uc_stack.ss_sp);
		free(n->uc);
	}
	if(n->waiters_queue) free_queue(n->waiters_queue); // UNLIKELY TO HAPPEN
	free(n);
}

void free_queue(struct queue *q)
{
	// printf("Please implements %s\n", __func__);
	if (!q)
		return;
	free_node(q->head);
	free(q);
}

struct queue *new_queue()
{
	struct queue *q = malloc(sizeof(struct queue));
	q->head = NULL;
	q->tail = NULL;
	return q;
}