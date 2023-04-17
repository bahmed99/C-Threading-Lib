#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <valgrind/valgrind.h>

#define STACK_SIZE 1024 * 32

int is_init = 0;

struct queue *thread_list;
struct queue *dirty_thread_list;
ucontext_t *cleaner_context;
int cleaner_valgrind_stackid;

// ucontext_t main_context;

void cleaner_context_function()
{
    for (;;)
    {
        struct node *to_clean = get_tail(dirty_thread_list);

        VALGRIND_STACK_DEREGISTER(to_clean->valgrind_stackid);
        free(to_clean->uc->uc_stack.ss_sp);
        free(to_clean->uc);

        struct node *new_n = get_head(thread_list);
        if (new_n)
        {
            if (new_n->next)
            {
                // printf("LINKING TO 2ND ELEMENT\n");
                new_n->uc->uc_link = new_n->next->uc;
            }
            // printf("WILL SWAPCONTEXT \n");
            // setcontext(new_n->uc);
            swapcontext(cleaner_context, new_n->uc);
        }
        else
        {
            // printf("EXITING\n");
            exit(0);
            // printf("[%s:%s] Error: Empty thread list after call: can't swapcontext\n", __FILE__, __func__);
        }
    }
}

void thread_init_if_necessary()
{
    if (is_init)
    {
        return;
    }
    is_init = 1;

    thread_list = new_queue();
    dirty_thread_list = new_queue();

    ucontext_t *main_context = malloc(sizeof(ucontext_t));
    getcontext(main_context);

    struct node *n = new_node(main_context);

    main_context->uc_link = NULL;
    main_context->uc_stack.ss_size = STACK_SIZE;
    main_context->uc_stack.ss_sp = malloc(main_context->uc_stack.ss_size);
    n->valgrind_stackid = VALGRIND_STACK_REGISTER(main_context->uc_stack.ss_sp,
                                                  main_context->uc_stack.ss_sp + main_context->uc_stack.ss_size);

    // printf("MainContext %p: sp: main_thread%p\n", main_context, main_context->uc_stack.ss_sp);

    add_tail(thread_list, n);

    cleaner_context = malloc(sizeof(ucontext_t));
    getcontext(cleaner_context);
    cleaner_context->uc_link = NULL;
    cleaner_context->uc_stack.ss_size = 1024;
    cleaner_context->uc_stack.ss_sp = malloc(cleaner_context->uc_stack.ss_size);
    cleaner_valgrind_stackid = VALGRIND_STACK_REGISTER(cleaner_context->uc_stack.ss_sp,
                                                       cleaner_context->uc_stack.ss_sp + cleaner_context->uc_stack.ss_size);

    makecontext(cleaner_context, cleaner_context_function, 0);

    atexit(thread_clean);
}

thread_t thread_self()
{
    thread_init_if_necessary();

    return get_head(thread_list);
}

void *thread_func_wrapper(void *(func)(void *), void *funcarg)
{
    // printf("Inside Wrapper\n");
    void *retval = func(funcarg);
    thread_exit(retval);
    // printf("SHOULD NOT BE CALLED\n");
    return NULL;
}

int thread_create(thread_t *newthread, void *(func)(void *), void *funcarg)
{
    thread_init_if_necessary();

    ucontext_t *current = malloc(sizeof(ucontext_t));

    struct node *n = new_node(current);

    getcontext(current);
    // current->uc_link = get_head(thread_list)->uc;
    current->uc_link = NULL;
    current->uc_stack.ss_size = STACK_SIZE;
    current->uc_stack.ss_sp = malloc(current->uc_stack.ss_size);

    n->valgrind_stackid = VALGRIND_STACK_REGISTER(current->uc_stack.ss_sp,
                                                  current->uc_stack.ss_sp + current->uc_stack.ss_size);
    // VALGRIND_STACK_REGISTER(current->uc_stack.ss_sp, STACK_SIZE);
    // printf("thread create context address: %p: sp: %p\n", current, current->uc_stack.ss_sp);

    makecontext(current, (void (*)(void))thread_func_wrapper, 2, func, funcarg);

    // struct thread_list_elem *e = new_thread_list_elem(current);
    *newthread = n;

    struct node *head = get_head(thread_list);
    if (!head->next)
    {
        head->uc->uc_link = n->uc;
    }

    add_tail(thread_list, n);

    return EXIT_SUCCESS;
}

int thread_yield()
{
    thread_init_if_necessary();

    // if (TAILQ_EMPTY(&thread_list))
    if (queue_empty(thread_list) || !get_head(thread_list)->next)
    {
        return EXIT_SUCCESS;
    }

    if (get_head(thread_list) == get_tail(thread_list))
    {
        return EXIT_SUCCESS;
    }

    struct node *n = pop_head(thread_list);

    add_tail(thread_list, n);

    // printf("SWAP3\n");
    // printf("BEFORE INVALID READ?\n");
    // printf("old node context address: %p: sp: %p\n", n->uc, n->uc->uc_stack.ss_sp);
    // printf("old node context address: %p\n", n->uc);
    // printf("old node context address: sp:%p\n", n->uc->uc_stack.ss_sp);
    // printf("AFTER INVALID READ?\n");

    struct node *new_n = get_head(thread_list);
    if (new_n->next)
    {
        new_n->uc->uc_link = new_n->next->uc;
    }

    // printf("new node context address: %p: sp: %p\n", new_n->uc, new_n->uc->uc_stack.ss_sp);
    // printf("here3-----------\n");

    // printf("SWAPCONTEXT RETURN %d\n", swapcontext(n->uc, new_n->uc));
    swapcontext(n->uc, new_n->uc);

    return EXIT_SUCCESS;
}

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
int thread_join(thread_t thread, void **retval)
{
    while (!thread->dirty)

    {
        // printf("JOIN->YIELD\n");
        thread_yield();
    }

    if (retval)
    {
        *retval = thread->retval;
    }

    // TAILQ_REMOVE(&dirty_thread_list, thread, nodes);
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
    // printf("---------RETVAL EXIT: %p\n", retval);
    struct node *n = pop_head(thread_list);
    n->retval = retval;
    n->dirty = 1;

    // printf("EXITING NODE CONTEXT %p, SSSP: %p\n", n->uc, n->uc->uc_stack.ss_sp);

    // makecontext(n->uc, NULL, 0);
    // free(n->uc->uc_stack.ss_sp);
    // free(n->uc);

    add_tail(dirty_thread_list, n);

    setcontext(cleaner_context);
    printf("SHOULD NEVER BE PRINTED !!!!!!!!!!!!!\n");
}

void thread_clean()
{
    // printf("Called %s\n", __func__);
    if (!is_init)
        return;

    free_queue(thread_list);
    free_queue(dirty_thread_list);

    VALGRIND_STACK_DEREGISTER(cleaner_context->uc_stack.ss_sp);
    free(cleaner_context->uc_stack.ss_sp);
    free(cleaner_context);

    // struct thread_list_elem *e = TAILQ_FIRST(&thread_list);

    // free_thread_list_elem(e);
    // free(main_context.uc_stack.ss_sp); // TODO
}

// struct thread_list_elem *new_thread_list_elem(ucontext_t context)
// {
//     struct thread_list_elem *e = malloc(sizeof(struct thread_list_elem));
//     e->uc = context;
//     e->dirty = 0;
//     e->retval = NULL;
//     return e;
// }

// void free_thread_list_elem(struct thread_list_elem *e)
// {
//     if (!e)
//         return;
//     if (e->uc.uc_stack.ss_sp)
//         free(e->uc.uc_stack.ss_sp);
//     free(e);
// }

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
    struct node *current_thread = get_head(thread_list);

    if (mutex->owner)
    {

        add_tail(mutex->waiting_mutex, current_thread);

        while (mutex->owner != current_thread)
        {
            remove_node(thread_list, current_thread);
            remove_node(thread_list, mutex->owner);
            add_head(thread_list, mutex->owner);
            swapcontext(current_thread->uc, mutex->owner->uc);
        }
    }
    else
    {
        mutex->owner = current_thread;
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
