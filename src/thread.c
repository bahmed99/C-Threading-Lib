#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

int is_init = 0;

struct queue* thread_list;
struct queue* dirty_thread_list;

// ucontext_t main_context;

void thread_init_if_necessary()
{
    if (is_init)
    {
        return;
    }
    is_init = 1;

    thread_list = new_queue();
    dirty_thread_list = new_queue();

    ucontext_t main_context;
    getcontext(&main_context);
    main_context.uc_link = NULL;
    main_context.uc_stack.ss_size = 64 * 1024;
    main_context.uc_stack.ss_sp = malloc(main_context.uc_stack.ss_size);
    struct node *n = new_node(main_context);

    add_tail(thread_list, n);

    atexit(thread_clean);
}

thread_t thread_self()
{
    thread_init_if_necessary();

    return get_head(thread_list);
}

void *thread_func_wrapper(void *(*func)(void *), void *funcarg)
{
    void *retval = func(funcarg);
    thread_exit(retval);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    // thread_init_if_necessary();

    ucontext_t current;

    getcontext(&current);
    current.uc_link = &get_head(thread_list)->uc;
    current.uc_stack.ss_size = 64 * 1024;
    current.uc_stack.ss_sp = malloc(current.uc_stack.ss_size);

    makecontext(&current, (void (*)(void))thread_func_wrapper, 2, func, funcarg);

    // struct thread_list_elem *e = new_thread_list_elem(current);
    struct node *n = new_node(current);
    *newthread = n;

    add_tail(thread_list, n);

    return EXIT_SUCCESS;
}

int thread_yield()
{
    thread_init_if_necessary();

    // if (TAILQ_EMPTY(&thread_list))
    if (queue_empty(thread_list))
    {
        return EXIT_SUCCESS;
    }

    struct node *n = pop_head(thread_list);

    add_tail(thread_list, n);

    // printf("SWAP3\n");
    swapcontext(&n->uc, &get_head(thread_list)->uc);

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
    free(thread->uc.uc_stack.ss_sp);
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
    // struct thread_list_elem *e = TAILQ_FIRST(&thread_list);
    // TAILQ_REMOVE(&thread_list, e, nodes);
    struct node *n = pop_head(thread_list);
    n->retval = retval;
    n->dirty = 1;

    // TAILQ_INSERT_TAIL(&dirty_thread_list, e, nodes);
    add_tail(dirty_thread_list, n);

    // struct thread_list_elem *new_e = TAILQ_FIRST(&thread_list);
    struct node *new_n = get_head(thread_list);
    if (n)
        swapcontext(&n->uc, &new_n->uc);
    else
        printf("[%s:%s] Error: Empty thread list after call: can't swapcontext", __FILE__, __func__);
}

void thread_clean()
{
    // printf("Please implements %s\n", __func__);
    if (!is_init)
        return;

    free_queue(thread_list);
    free_queue(dirty_thread_list);

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
}

int thread_mutex_destroy(thread_mutex_t *mutex)
{
}

int thread_mutex_lock(thread_mutex_t *mutex)
{
}

int thread_mutex_unlock(thread_mutex_t *mutex)
{
}
