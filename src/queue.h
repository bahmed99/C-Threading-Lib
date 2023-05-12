#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "thread.h"

struct queue
{
	struct node *head;
	struct node *tail;
};

struct node
{
	struct node *next;
	struct node *prec;
	ucontext_t *uc;
	void *retval;
	int dirty;
	int valgrind_stackid;
#if SCHEDULING_POLICY == 1
	int priority;
#endif

	struct queue *waiters_queue;
	int waiter_count;
};

void add_tail(struct queue *q, struct node *n);

void add_head(struct queue *q, struct node *n);

struct node *get_head(struct queue *q);

struct node *get_tail(struct queue *q);

struct node *pop_head(struct queue *q);

void remove_node(struct queue *q, struct node *n);

int queue_empty(struct queue *q);

void append_queue(struct queue *q1, struct queue *q2);

struct queue *new_queue();
void free_queue(struct queue *q);

struct node *new_node(ucontext_t *context);
void free_node(struct node *n);

int detect_deadlock(struct node *, struct node *);

#endif //!__QUEUE_H__