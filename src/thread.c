#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <valgrind/valgrind.h>


#define STACK_SIZE 1024 * 12

// This variable is used by thread_init_if_necessary() function in order to track if the library has already been initialized
int is_init = 0;

#if SCHEDULING_POLICY == 0
// This queue store the current threads
// They are ordered by scheduling order
// So the first one in the queue is the currently running thread
struct queue *thread_list;
#elif SCHEDULING_POLICY == 1
// This array will contains at index i a queue of threads list with the priority i
struct queue* threads_priority_array[10];
int currrent_highest_priority = 0;
#endif

// We describe as dirty a thread that returned a value with the "return" keyword or the thread_exit() function
// We use this variable to store the last dirty thread in order to free its context and stack
// (we don't do it if it's the main context)
// -> Indeed, the main context is a bit special because usually the current context can't free his own stack without causing invalid reads
// But it seems that the main context can -> well played to him
struct node *dirty_thread;

// We associate for each context (main, cleaner and all those in struct node*) a valgrind_stackid
// This id is used by VALGRIND_STACK_REGISTER and VALGRIND_STACK_DEREGISTER in order to help valgrind for tracking context/stack switching

// We need to store the main context in order to free it while exiting the program
ucontext_t *main_context;
int main_valgrind_stackid;
int main_thread_joined = 0;

void free_context(struct node *n)
{
    VALGRIND_STACK_DEREGISTER(n->valgrind_stackid);
    free(n->uc->uc_stack.ss_sp);
    free(n->uc);
    n->uc = NULL;
}

void clean_last_dirty_thread()
{
    if (dirty_thread)
    {
        // We need to make sure that we are not freeing main context
        // if (main_context != to_clean->uc && NULL != to_clean->uc)
        if (main_context != dirty_thread->uc)
        {
            free_context(dirty_thread);
        }
        else
        {
            main_thread_joined = 1;
        }
        dirty_thread = NULL;
    }
}



#if SCHEDULING_POLICY == 0
void append_node_to_queue(struct node* n) {
    add_tail(thread_list, n);
}

void init_thread_lists()
{
    thread_list = new_queue();
}

struct node* create_thread_node(ucontext_t* context) {
    return new_node(context);
}

struct node* get_queue_head() {
    return get_head(thread_list);
}

struct node* pop_queue_head() {
    return pop_head(thread_list);
}

void append_queue_to_queue(struct queue* src) {
    append_queue(thread_list, src);
}

void free_threads() {
    free_queue(thread_list);
}

#elif SCHEDULING_POLICY == 1
void init_thread_lists()
{
    for (size_t i = 0; i < 10; i++)
    {
        threads_priority_array[i] = new_queue();
    }
    

}

void append_node_to_queue(struct node* n) {
    add_tail(threads_priority_array[n->priority], n);
}

struct node* create_thread_node(ucontext_t* context) {
    struct node* n = new_node(context);
    n->priority = 0;

    return n;

}

struct node* get_queue_head() {
    return get_head(threads_priority_array[currrent_highest_priority]);
}


struct node* pop_queue_head() {
    return pop_head(threads_priority_array[currrent_highest_priority]);
}

void append_queue_to_queue(struct queue* src) {
    append_queue(threads_priority_array[currrent_highest_priority], src);
}

void free_threads() {
    for(int i = 0; i < 10; i++) {
        if(!queue_empty(threads_priority_array[i])) {
            printf("Index: %d \n", i);
            printf("Head: %p \n", get_queue_head(threads_priority_array[i]));
        }
        free_queue(threads_priority_array[i]);
    }
}

void update_highest_priority() {
    while(queue_empty(threads_priority_array[currrent_highest_priority])) {
        currrent_highest_priority -= 1;
        if(currrent_highest_priority < 0) {
            printf("Something wrong happenned ! \n");
            currrent_highest_priority = 0;
        }
    }
}

#endif




// Initialize our library if it was not done before (this information is saved with is_init variable)
void thread_init_if_necessary()
{
    if (is_init)
    {
        return;
    }
    is_init = 1;

    // Initialize the thread queue(s)
    init_thread_lists();

    // Initialize main context and its associated node
    main_context = malloc(sizeof(ucontext_t));
    getcontext(main_context);
    struct node *n = create_thread_node(main_context);

    // Setup the main context and allocates memory for its stack (and indicated valgrind that we do so)
    main_context->uc_link = NULL;
    main_context->uc_stack.ss_size = 1024;
    main_context->uc_stack.ss_sp = malloc(main_context->uc_stack.ss_size);
    main_valgrind_stackid = VALGRIND_STACK_REGISTER(main_context->uc_stack.ss_sp, main_context->uc_stack.ss_sp + main_context->uc_stack.ss_size);
    n->valgrind_stackid = main_valgrind_stackid;

    // Add the main_thread_node to the thread list
    // -> The main thread is at this point the only and currently executed thread
    append_node_to_queue(n);

    // Set thread_clean() as a function to be executed by the program exiting
    atexit(thread_clean);
}

thread_t thread_self()
{
    thread_init_if_necessary();

    // The currently executed thread is the first one in thread_list
    return get_queue_head();
}

// This function is used to wrap any function given to thread_create in order to make sure that they call thread_exit
void *thread_func_wrapper(void *(func)(void *), void *funcarg)
{
    //clean_last_dirty_thread();
    void *retval = func(funcarg);

    thread_exit(retval);

    return (void *)0xdeadbeef; // This line should not be executed but the compiler will happily read it
}

int thread_create(thread_t *newthread, void *(func)(void *), void *funcarg)
{
    thread_init_if_necessary();

    // Creates the context and its associated node
    ucontext_t *current = malloc(sizeof(ucontext_t));
    struct node *n = create_thread_node(current);

    // Setup the context and allocates memory for its stack (and tell valgrind that we do so)
    getcontext(current);
    current->uc_link = NULL;
    current->uc_stack.ss_size = STACK_SIZE;
    current->uc_stack.ss_sp = malloc(current->uc_stack.ss_size);
    n->valgrind_stackid = VALGRIND_STACK_REGISTER(current->uc_stack.ss_sp, current->uc_stack.ss_sp + current->uc_stack.ss_size);

    // Associates the given function (wrapped with thread_func_wrapper) to the context
    makecontext(current, (void (*)(void))thread_func_wrapper, 2, func, funcarg);

    // Returns the thread to the given location
    *newthread = n;


    // Add to the thread list as the last thread to execute
    append_node_to_queue(n);

    return EXIT_SUCCESS;
}


// Gives execution to the next thread in queue
int thread_yield()
{
    thread_init_if_necessary();

    // Little optimization -> don't yield if there is only one thread in our list (yielding to itself is useless)
    if (!get_queue_head()->next)
    {
        return EXIT_SUCCESS;
    }

    struct node *n = pop_queue_head();
    append_node_to_queue(n);
    int res = swapcontext(n->uc, get_queue_head()->uc);
    clean_last_dirty_thread();
    return res;
}

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
int thread_join(thread_t thread, void **retval)
{
    if (detect_deadlock(get_queue_head(), thread))
    {
        return -1;
    }

    // Putting the waiting thread into the given thread waiters_queue
    if (!thread->dirty)
    {
        struct node *n = pop_queue_head();
        if (thread->waiters_queue == NULL)
        {
            thread->waiters_queue = new_queue();
        }
        add_tail(thread->waiters_queue, n);

#if SCHEDULING_POLICY == 1
        remove_node(threads_priority_array[currrent_highest_priority], thread);
        thread->priority = thread->priority == 9 ? 9 : thread->priority + 1;
        if(thread->priority > currrent_highest_priority) {
            currrent_highest_priority = thread->priority;
        }
        append_node_to_queue(thread);
#endif

        swapcontext(n->uc, get_queue_head()->uc);
        clean_last_dirty_thread();
    }

    // From now on, thread->dirty should be equal to 1
    // (Thanks to the waiters queue functionality, see thread_exit() for more informations)

    if (retval)
    {
        *retval = thread->retval;
    }

    free(thread);

    return EXIT_SUCCESS;
}

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 *
 * L'attribut noreturn aide le compilateur à optimiser le code de
 * l'application (élimination de code mort). Attention à ne pas mettre
 * cet attribut dans votre interface tant que votre thread_exit()
 * n'est pas correctement implémenté (il ne doit jamais retourner).
 */
void thread_exit(void *retval)
{
    struct node *n = pop_queue_head();
    n->retval = retval;
    n->dirty = 1;

    dirty_thread = n;

    // Waiters queue functionality
    // All threads that called thread_join will be put on the waiters_queue of this thread (and out of the classic thread_list)
    // And when this thread call thread_exit, waiters are put back into thread_list
    if (n->waiters_queue)
    {
        append_queue_to_queue(n->waiters_queue);
        free(n->waiters_queue);
        n->waiters_queue = NULL;
    }

#if SCHEDULING_POLICY == 1
    update_highest_priority();
#endif
    struct node *new_n = get_queue_head();
    if (new_n)
    {
        swapcontext(n->uc, new_n->uc);
    }
    else if (main_thread_joined)
    {
        // No thread to execute so we execute the main_context that should be inside thread_exit() method
        setcontext(main_context);
    }

    // We access this only if the main thread is joined by another one -> but we still need to go back to it to properly end the process
    exit(0);
}

void thread_clean()
{
    if (!is_init)
        return;

    free_threads();

    if (main_thread_joined)
    {
        free_context(dirty_thread);
        free(dirty_thread);
    }
    else if (dirty_thread)
    {
        free(dirty_thread);
    }

    VALGRIND_STACK_DEREGISTER(main_valgrind_stackid);
    free(main_context->uc_stack.ss_sp);
    free(main_context);
}

int thread_mutex_init(thread_mutex_t *mutex)
{

    mutex->waiting_mutex = new_queue();
    mutex->owner = NULL;

    return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex)
{
    free_queue(mutex->waiting_mutex);
    return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex)
{

    if (mutex->owner)
    {
        struct node *current_thread = pop_queue_head();

        add_tail(mutex->waiting_mutex, current_thread);
        swapcontext(current_thread->uc, get_queue_head()->uc);
        clean_last_dirty_thread();
    }
    else
    {
        mutex->owner = get_queue_head();
    }
}

int thread_mutex_unlock(thread_mutex_t *mutex)
{

    if (!queue_empty(mutex->waiting_mutex))
    {
        struct node *head = pop_head(mutex->waiting_mutex);
        append_node_to_queue(head);
        mutex->owner = head;
    }
    else
    {
        mutex->owner = NULL;
    }
    return 0;
}
