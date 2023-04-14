#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

int is_init = 0;

TAILQ_HEAD(thread_list_s, thread_list_elem)
thread_list;

TAILQ_HEAD(dirty_thread_list_s, thread_list_elem)
dirty_thread_list;

ucontext_t *main_context = NULL;

void thread_init_if_necessary()
{
    if (is_init)
    {
        return;
    }
    is_init = 1;
    TAILQ_INIT(&thread_list);
    TAILQ_INIT(&dirty_thread_list);
    main_context = malloc(sizeof(ucontext_t));
    getcontext(main_context);
    struct thread_list_elem *e = malloc(sizeof(struct thread_list_elem));
    e->dirty = 0;
    e->uc = main_context;
    e->retval = NULL;
    main_context->uc_link = NULL;
    main_context->uc_stack.ss_size = 64 * 1024;
    main_context->uc_stack.ss_sp = malloc(main_context->uc_stack.ss_size);

    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    atexit(thread_clean);
}

thread_t thread_self()
{
    thread_init_if_necessary();

    return TAILQ_FIRST(&thread_list);
}

void* thread_func_wrapper(void *(*func)(void *), void *funcarg) {
    void* retval = func(funcarg);
    thread_exit(retval);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    thread_init_if_necessary();

    ucontext_t *current = malloc(sizeof(ucontext_t));

    getcontext(current);
    current->uc_link = TAILQ_FIRST(&thread_list)->uc;
    current->uc_stack.ss_size = 64 * 1024;
    current->uc_stack.ss_sp = malloc(current->uc_stack.ss_size);

    makecontext(current, (void (*)(void))thread_func_wrapper, 2, func, funcarg);

    struct thread_list_elem *e = malloc(sizeof(struct thread_list_elem));
    *newthread = e;
    e->uc = current;
    e->dirty = 0;
    e->retval = NULL;

    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    return EXIT_SUCCESS;
}

int thread_yield()
{
    thread_init_if_necessary();

    if (TAILQ_EMPTY(&thread_list))
    {
        return EXIT_SUCCESS;
    }

    struct thread_list_elem *e = TAILQ_FIRST(&thread_list);

    TAILQ_REMOVE(&thread_list, e, nodes);
    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    // printf("SWAP3\n");
    swapcontext(e->uc, TAILQ_FIRST(&thread_list)->uc);

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

    TAILQ_REMOVE(&dirty_thread_list, thread, nodes);
    free(thread->uc->uc_stack.ss_sp);
    free(thread->uc);
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
    struct thread_list_elem *e = TAILQ_FIRST(&thread_list);
    e->retval = retval;
    e->dirty = 1;
    TAILQ_REMOVE(&thread_list, e, nodes);
    TAILQ_INSERT_TAIL(&dirty_thread_list, e, nodes);

    struct thread_list_elem *new_e = TAILQ_FIRST(&thread_list);
    if (new_e)
    {
        // printf("SWAP1\n");
        swapcontext(e->uc, new_e->uc);
    }
}

void thread_clean()
{
    if(!is_init) {
        return;
    }
    
    struct thread_list_elem *e = TAILQ_FIRST(&thread_list);
    free(e->uc->uc_stack.ss_sp);
    free(e->uc);
    free(e);
}

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
