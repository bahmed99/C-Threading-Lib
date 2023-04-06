#include "thread.h"
#include "ucontext.h"
#include "queue.h"
#include "stdlib.h"

// typedef struct thread_t {
//     ucontext_t* uc;
// } thread_t;



SLIST_HEAD(thread_list, thread_t) thread_list = SLIST_HEAD_INITIALIZER(thread_list); // Init the queue

thread_t thread_self(void) { // Thread structure
    ucontext_t current;
    getcontext(&current);
    thread_t t = { &current};
    return t;
}



int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {

    ucontext_t current, main;

    getcontext(&current);
    getcontext(&main);

    current.uc_link = &main;

    makecontext(&current, func, funcarg);

    return 0;
}

 int thread_yield(void){

 }


int thread_join(thread_t thread, void **retval){
    
}


void thread_exit(void *retval) {
    
}




int thread_mutex_init(thread_mutex_t *mutex){

}
int thread_mutex_destroy(thread_mutex_t *mutex){

}
int thread_mutex_lock(thread_mutex_t *mutex){
    
}
int thread_mutex_unlock(thread_mutex_t *mutex){

}