#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>


struct thread_list_elem {
    thread_t thread;
    TAILQ_ENTRY(thread_list_elem) nodes;
};

TAILQ_HEAD(thread_list_s, thread_list_elem) thread_list;

thread_t thread_self() {
    return TAILQ_FIRST(&thread_list)->thread;
}


void* thread_init() {
    TAILQ_INIT(&thread_list);
}


/**
 * The purpose of this function is to append to the thread list an new thread. (It will be executed
 * with thread_yield afterwards)
*/
int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {

    ucontext_t* current, *main = malloc(sizeof(ucontext_t));

    getcontext(current);
    getcontext(main);

    current->uc_link = main;

    makecontext(current, func, funcarg);


    struct thread_list_elem* e = malloc(sizeof(struct thread_list_elem));
    e->thread = *newthread;
    e->thread->uc = current;

    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    return EXIT_SUCCESS;
}

int thread_yield() {

    if(TAILQ_EMPTY(&thread_list)) {
        return EXIT_SUCCESS;
    }

    struct thread_list_elem* e = TAILQ_FIRST(&thread_list);

    TAILQ_REMOVE(&thread_list, e, nodes);
    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    ucontext_t t;
    swapcontext(&t, TAILQ_FIRST(&thread_list)->thread->uc);

    return EXIT_SUCCESS; // Should not be read
}

