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

// This queue store the current threads
// They are ordered by scheduling order
// So the first one in the queue is the currently running thread
struct queue *thread_list;

// We describe as dirty a thread that returned a value with the "return" keyword or the thread_exit() function
// A thread in this queue has always his dirty property to 1
// Threads in this queue has a freed context and freed stack (ss_sp) except for the main context
// -> Indeed, the main context is a bit special because usually the current context can't free his own stack without causing invalid reads
// But it seems that the main context can -> well played to him
struct queue *dirty_thread_list;

// We associate for each context (main, cleaner and all those in struct node*) a valgrind_stackid
// This id is used by VALGRIND_STACK_REGISTER and VALGRIND_STACK_DEREGISTER in order to help valgrind for tracking context/stack switching

// We need to store the main context in order to free it while exiting the program
ucontext_t *main_context;
int main_valgrind_stackid;
int main_thread_joined = 0;

// The cleaner context is used in pairs with the cleaner_context_function() in order to free context and stack from a dirty thread
ucontext_t *cleaner_context;
int cleaner_valgrind_stackid;

void clean_last_dirty_thread()
{
    if (!queue_empty(dirty_thread_list))
    {
        // Get the last thread that called thread_exit()
        struct node *to_clean = get_tail(dirty_thread_list);
        // printf("-> %p | %p\n", to_clean->uc, to_clean->uc->uc_stack);

        // We need to make sure that we are not freeing main context
        if (main_context != to_clean->uc && NULL != to_clean->uc)
        {
            VALGRIND_STACK_DEREGISTER(to_clean->valgrind_stackid);
            free(to_clean->uc->uc_stack.ss_sp);
            free(to_clean->uc);
            to_clean->uc = NULL;
        }
        else
        {
            // printf("MAIN THREAD JOINED\n");
            main_thread_joined = 1;
        }
    }
}

// Only one loop step of this function should be executed each time we setcontext(cleaner_context)
// It will free the context and stack from the last added thread to dirty_thread_list
// (ie the last thread that returned or called thread_exit -> almost the same thing thanks to our thread_func_wrapper())
void cleaner_context_function()
{
    for (;;)
    {
        // clean_last_dirty_thread();

        // Get the next thread to execute
        struct node *new_n = get_head(thread_list);
        if (new_n)
        {
            // We link the second thread in our thread list to uc_link
            if (new_n->next)
            {
                new_n->uc->uc_link = new_n->next->uc;
            }
            swapcontext(cleaner_context, new_n->uc);
        }
        else
        {
            // No thread to execute so we execute the main_context that should be inside thread_exit() method
            setcontext(main_context);
        }
    }
}

// Initialize our library if it was not done before (this information is saved with is_init variable)
void thread_init_if_necessary()
{
    if (is_init)
    {
        return;
    }
    is_init = 1;

    // Initialize the two thread queues
    thread_list = new_queue();
    dirty_thread_list = new_queue();

    // Initialize main context and its associated node
    main_context = malloc(sizeof(ucontext_t));
    getcontext(main_context);
    struct node *n = new_node(main_context);

    // Setup the main context and allocates memory for its stack (and indicated valgrind that we do so)
    main_context->uc_link = NULL;
    main_context->uc_stack.ss_size = STACK_SIZE;
    main_context->uc_stack.ss_sp = malloc(main_context->uc_stack.ss_size);
    main_valgrind_stackid = VALGRIND_STACK_REGISTER(main_context->uc_stack.ss_sp, main_context->uc_stack.ss_sp + main_context->uc_stack.ss_size);
    n->valgrind_stackid = main_valgrind_stackid;

    // Add the main_thread_node to the thread list
    // -> The main thread is at this point the only and currently executed thread
    add_tail(thread_list, n);

    // Initialize cleaner context
    // (we don't need a node for it since this context won't be stored as a classic thread in our lists)
    cleaner_context = malloc(sizeof(ucontext_t));
    getcontext(cleaner_context);
    // As for main_context we allocates memory for the cleaner context stack
    cleaner_context->uc_link = NULL;
    cleaner_context->uc_stack.ss_size = 1024;
    cleaner_context->uc_stack.ss_sp = malloc(cleaner_context->uc_stack.ss_size);
    cleaner_valgrind_stackid = VALGRIND_STACK_REGISTER(cleaner_context->uc_stack.ss_sp, cleaner_context->uc_stack.ss_sp + cleaner_context->uc_stack.ss_size);

    // Associates the cleaner_context_function to the cleaner context
    makecontext(cleaner_context, cleaner_context_function, 0);

    // Set thread_clean() as a function to be executed by the program exiting
    atexit(thread_clean);
}

thread_t thread_self()
{
    thread_init_if_necessary();

    // The currently executed thread is the first one in thread_list
    return get_head(thread_list);
}

// This function is used to wrap any function given to thread_create in order to make sure that they call thread_exit
void *thread_func_wrapper(void *(func)(void *), void *funcarg)
{
    clean_last_dirty_thread();

    void *retval = func(funcarg);
    thread_exit(retval);

    return (void *)0xdeadbeef; // This line should not be executed but the compiler will happily read it
}

int thread_create(thread_t *newthread, void *(func)(void *), void *funcarg)
{
    thread_init_if_necessary();

    // Creates the context and its associated node
    ucontext_t *current = malloc(sizeof(ucontext_t));
    struct node *n = new_node(current);

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

    // Link the new thread to the one that created it (default behaviour for our implementation)
    struct node *head = get_head(thread_list);
    if (!head->next)
    {
        head->uc->uc_link = n->uc;
    }

    // Add to the thread list as the last thread to execute
    add_tail(thread_list, n);

    return EXIT_SUCCESS;
}

// Internal tiny function that links the dst thread to it's next if it has one
int link_and_context_swap(struct node *src, struct node *dst)
{
    if (dst->next)
    {
        dst->uc->uc_link = dst->next->uc;
    }

    int res = swapcontext(src->uc, dst->uc);

    clean_last_dirty_thread();

    return res;
}

// Gives execution to the next thread in queue
int thread_yield()
{
    thread_init_if_necessary();

    // This test should not pass in a normal execution (was useful for testing)
    // We may delete it later for increased performance
    if (queue_empty(thread_list))
    {
        return EXIT_FAILURE;
    }

    // Little optimization -> don't yield if there is only one thread in our list (yielding to itself is useless)
    if (!get_head(thread_list)->next)
    {
        return EXIT_SUCCESS;
    }

    struct node *n = pop_head(thread_list);
    add_tail(thread_list, n);

    return link_and_context_swap(n, get_head(thread_list));
}

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
int thread_join(thread_t thread, void **retval)
{
    if (detect_deallock(get_head(thread_list), thread))
    {
        return -1;
    }

    // Putting the waiting thread into the given thread waiters_queue
    if (!thread->dirty)
    {
        struct node *n = pop_head(thread_list);
        if (thread->waiters_queue == NULL)
        {
            thread->waiters_queue = new_queue();
        }
        add_tail(thread->waiters_queue, n);

        link_and_context_swap(n, get_head(thread_list));
    }

    // From now on, thread->dirty should be equal to 1
    // (Thanks to the waiters queue functionality, see thread_exit() for more informations)

    if (retval)
    {
        *retval = thread->retval;
    }

    remove_node(dirty_thread_list, thread);
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
    struct node *n = pop_head(thread_list);
    n->retval = retval;
    n->dirty = 1;

    add_tail(dirty_thread_list, n);

    // Waiters queue functionality
    // All threads that called thread_join will be put on the waiters_queue of this thread (and out of the classic thread_list)
    // And when this thread call thread_exit, waiters are put back into thread_list
    if (n->waiters_queue)
    {
        append_queue(thread_list, n->waiters_queue);
        free(n->waiters_queue);
        n->waiters_queue = NULL;
    }

    struct node *new_n = get_head(thread_list);
    if (new_n)
    {
        link_and_context_swap(n, new_n);
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

    free_queue(thread_list);
    free_queue(dirty_thread_list);

    VALGRIND_STACK_DEREGISTER(cleaner_valgrind_stackid);
    free(cleaner_context->uc_stack.ss_sp);
    free(cleaner_context);

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
        struct node *current_thread = pop_head(thread_list);

        add_tail(mutex->waiting_mutex, current_thread);
        link_and_context_swap(current_thread, get_head(thread_list));
    }
    else
    {
        mutex->owner = get_head(thread_list);
    }
}

int thread_mutex_unlock(thread_mutex_t *mutex)
{

    if (!queue_empty(mutex->waiting_mutex))
    {
        struct node *head = pop_head(mutex->waiting_mutex);
        add_tail(thread_list, head);
        mutex->owner = head;
    }
    else
    {
        mutex->owner = NULL;
    }
    return 0;
}
