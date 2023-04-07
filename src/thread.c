#include "thread.h"
#include <ucontext.h>
#include "queue.h"
#include <stdlib.h>

struct thread_list_elem
{
    thread_t thread;
    TAILQ_ENTRY(thread_list_elem)
    nodes;
};

TAILQ_HEAD(thread_list_s, thread_list_elem)
thread_list;

void *thread_init()
{
    TAILQ_INIT(&thread_list);
}

thread_t thread_self()
{
    return TAILQ_FIRST(&thread_list)->thread;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{

    ucontext_t *current, *main = malloc(sizeof(ucontext_t));

    getcontext(current);
    getcontext(main);

    current->uc_link = main;

    makecontext(current, (void (*)(void))func, 1, funcarg);

    struct thread_list_elem *e = malloc(sizeof(struct thread_list_elem));
    e->thread = *newthread;
    e->thread->uc = current;

    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    return EXIT_SUCCESS;
}

int thread_yield()
{

    if (TAILQ_EMPTY(&thread_list))
    {
        return EXIT_SUCCESS;
    }

    struct thread_list_elem *e = TAILQ_FIRST(&thread_list);

    TAILQ_REMOVE(&thread_list, e, nodes);
    TAILQ_INSERT_TAIL(&thread_list, e, nodes);

    ucontext_t t;
    swapcontext(&t, TAILQ_FIRST(&thread_list)->thread->uc);

    return EXIT_SUCCESS; // Should not be read
}

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
int thread_join(thread_t thread, void **retval)
{
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
