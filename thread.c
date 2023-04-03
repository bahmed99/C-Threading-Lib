#include "thread.h"
#include "ucontext.h"
#include "queue.h"
#include "stdlib.h"

typedef struct thread_t {
    ucontext_t* uc;
} thread_t;



SLIST_HEAD(thread_list, thread_t) thread_list = SLIST_HEAD_INITIALIZER(thread_list); // Init the queue

thread_t thread_self(void) { // Thread structure
    ucontext_t current;
    getcontext(&current);
    thread_t t = {.uc = &current};
    return t;
}


/**
 * The purpose of this function is to append to the thread list an new thread. (It will be executed
 * with thread_yield afterwards)
*/
int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {

    ucontext_t current, main;

    getcontext(&current);
    getcontext(&main);

    current.uc_link = &main;

    makecontext(&current, func, funcarg);

    return 0;
}