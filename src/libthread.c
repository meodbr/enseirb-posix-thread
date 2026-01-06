#define _XOPEN_SOURCE 700

#include "thread.h"
#include "black_red_tree.h"
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <ucontext.h>
#include <time.h>
#include <valgrind/valgrind.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#define MAIN_THREAD_ID 1
#define MAX_YIELD_UNTIL_REORDER 4
#define MAX_CPU_TIME_UNTIL_REORDER 2000 * 1000
#define PREEMPT_TIME_INTERVAL 2100 // in us
#define MULTIPLIERS_VALUES 10000000, 7943282, 6309573, 5011872, 3981071, 3162277, 2511886, 1995262, 1584893, 1258925, 1000000, 794328, 630957, 501187, 398107, 316227, 251188, 199526, 158489, 125892, 100000, 79432, 63095, 50118, 39810, 31622, 25118, 19952, 15848, 12589, 10000, 7943, 6309, 5011, 3981, 3162, 2511, 1995, 1584, 1258

#ifdef USE_PREEMPTION
#define STACK_SIZE 16 * 1024
#define USE_PREEMPTION 1
#else
#define STACK_SIZE 1024 * 1024
#define USE_PREEMPTION 0
#endif

typedef enum thread_state
{
    READY,
    TERMINATED
} thread_state;

struct thread_struct;

BRTREE_ENTRY_DEF(thread_struct);
BRTREE_DEF(thread_struct);

typedef struct thread_struct
{
    thread_state state;
    int id;
    int priority;
    ucontext_t context;
    void *retval;
    int valgrind_stack_id;
    int nb_yields_since_reorder;
    long long cpu_time_since_reorder;
    struct thread_struct *who_is_waiting_for_me, *next_in_mutex_queue;
    BRTREE_ENTRY(thread_struct)
    brtree_entry; // the name should always be brtree_entry
} thread_struct;

static thread_struct *current_thread;
static int next_id = MAIN_THREAD_ID;
static unsigned long long start_time = 0;
static BRTREE(thread_struct) threads = BRTREE_INITIALIZER;
static thread_struct main_thread_data;
static thread_struct * main_thread = &main_thread_data;
static const long long priority_multipliers[40] = { MULTIPLIERS_VALUES };
static struct sigaction preempt_siga;
static struct itimerval preempt_timer;
static int preempt_lock = 0;

#define PREEMPT_LOCK preempt_lock = 1
#define PREEMPT_UNLOCK preempt_lock = 0
#define PREEMPT_IS_LOCKED (preempt_lock == 1)

// Inline assembly to read the Time Stamp Counter (TSC)

uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
    
static void preempt_handler(int) { 
    if (!PREEMPT_IS_LOCKED) { 
        thread_yield();
    }
}

__attribute__((constructor)) void init()
{
    main_thread->id = next_id++;
    main_thread->priority = 20;
    main_thread->state = READY;
    main_thread->retval = NULL;
    main_thread->nb_yields_since_reorder = 0;
    main_thread->cpu_time_since_reorder = 0;
    main_thread->who_is_waiting_for_me = NULL;
    main_thread->next_in_mutex_queue = NULL;
    main_thread->context.uc_stack.ss_sp = NULL;
    main_thread->valgrind_stack_id = VALGRIND_STACK_REGISTER(main_thread->context.uc_stack.ss_sp, main_thread->context.uc_stack.ss_sp + STACK_SIZE);
    getcontext(&main_thread->context);
    BRTREE_ENTRY_INITIALIZE(main_thread, 0);
    BRTREE_INSERT(main_thread, &threads, thread_struct);

    // Initialisation de la préemption
    preempt_siga.sa_handler = preempt_handler;
    sigemptyset(&preempt_siga.sa_mask);
    preempt_siga.sa_flags = 0;
    sigaction(SIGALRM, &preempt_siga, NULL);

    preempt_timer.it_value.tv_sec = 0;
    preempt_timer.it_value.tv_usec = PREEMPT_TIME_INTERVAL;
    preempt_timer.it_interval = preempt_timer.it_value;

    current_thread = main_thread;
    if(USE_PREEMPTION) setitimer(ITIMER_REAL, &preempt_timer, NULL);
}

__attribute__((destructor)) void destroy()
{
    if (main_thread->who_is_waiting_for_me != NULL)
    {
        free(main_thread->who_is_waiting_for_me->context.uc_stack.ss_sp);
        free(main_thread->who_is_waiting_for_me);
    }
}

/* recuperer l'identifiant du thread courant.
 */
extern thread_t thread_self(void)
{
    return current_thread;
}

void thread_function_wrapper(void *(*function)(void *), void *arg)
{
    start_time = rdtsc();
    PREEMPT_UNLOCK;
    thread_exit(function(arg));
}

/* creer un nouveau thread qui va exécuter la fonction func avec l'argument funcarg.
 * renvoie 0 en cas de succès, -1 en cas d'erreur.
 */
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
    // Allocation de la structure du thread
    thread_struct *new_thread = malloc(sizeof(thread_struct));
    if (new_thread == NULL)
        return -1;

    // Initialisation de la structure du thread
    new_thread->id = next_id++;
    new_thread->priority = 20;
    new_thread->state = READY;
    new_thread->nb_yields_since_reorder = 0;
    new_thread->cpu_time_since_reorder = 0;
    new_thread->retval = NULL;
    new_thread->who_is_waiting_for_me = NULL;
    new_thread->next_in_mutex_queue = NULL;

    // Gestion du contexte et de la pile du thread créé
    new_thread->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->valgrind_stack_id = VALGRIND_STACK_REGISTER(new_thread->context.uc_stack.ss_sp, new_thread->context.uc_stack.ss_sp + STACK_SIZE);
    new_thread->context.uc_link = NULL;
    getcontext(&new_thread->context); // recupere le contexte actuel

    if (new_thread->context.uc_stack.ss_sp == NULL)
    {
        free(new_thread);
        return -1;
    }
    makecontext(&new_thread->context, (void (*)(void))thread_function_wrapper, 2, func, funcarg);

    *newthread = new_thread;

    // Ajout du thread à la liste des threads
    BRTREE_ENTRY_INITIALIZE(new_thread, 0);

    PREEMPT_LOCK;
    BRTREE_INSERT(new_thread, &threads, thread_struct);
    PREEMPT_UNLOCK;

    return 0;
}

static int last_yield() {
    thread_struct *next_thread = main_thread;
    if (current_thread != main_thread)
    {
        main_thread->who_is_waiting_for_me = current_thread;
        thread_struct *save_thread = current_thread;
        current_thread = next_thread;
        swapcontext(&save_thread->context, &current_thread->context);
    }
    return 0;
}

/*
 * passer la main à un autre thread.
 */
extern int thread_yield(void)
{
    PREEMPT_LOCK;
    // Storing cpu time used since last yield
    unsigned long long end_time = rdtsc();
    current_thread->cpu_time_since_reorder += (BRTREE_KEY(current_thread) == 0 && current_thread->cpu_time_since_reorder == 0 ? 
                                                1 : (long long)(end_time - start_time));
    current_thread->nb_yields_since_reorder++;
    start_time = end_time;

    // Deciding whether give hand or not
    int is_current_schedulable = BRTREE_IS_IN_TREE(current_thread, &threads);
    if (current_thread->nb_yields_since_reorder < next_id-1 &&
        current_thread->cpu_time_since_reorder < MAX_CPU_TIME_UNTIL_REORDER &&
        is_current_schedulable) 
    { // if threshold hasn't been exceeded
        PREEMPT_UNLOCK;
        return 0;
    } else { // if threshold has been exceeded
        BRTREE_KEY(current_thread) += ((current_thread->cpu_time_since_reorder) * priority_multipliers[current_thread->priority]);
        current_thread->nb_yields_since_reorder = 0;
        current_thread->cpu_time_since_reorder = 0;
        if (is_current_schedulable) {
            BRTREE_REORDER(current_thread, &threads, thread_struct); 
        }
    }

    if (BRTREE_EMPTY(&threads)) return last_yield();

    // Giving hand to thread who has the less cpu time
    thread_struct *next_thread, *save_thread = current_thread;
    BRTREE_GET_SMALLER_KEY(&threads, next_thread);
    current_thread = next_thread;
    if (current_thread != save_thread) swapcontext(&save_thread->context, &current_thread->context);
    start_time = rdtsc();
    PREEMPT_UNLOCK;
    return 0;
}

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
extern int thread_join(thread_t thread, void **retval)
{
    thread_struct *thread_to_join = (thread_struct *)thread;
    if (thread_to_join == NULL)
        return -1;
    
    thread_struct *tmp = current_thread;
    while (tmp->who_is_waiting_for_me != NULL && tmp != thread_to_join) tmp = tmp->who_is_waiting_for_me;
    if (tmp == thread_to_join)
        return EDEADLK;

    PREEMPT_LOCK;
    if (thread_to_join->state != TERMINATED)
    {
        thread_to_join->who_is_waiting_for_me = current_thread;
        BRTREE_ERASE(current_thread, &threads, thread_struct);
        thread_yield();
    }
    PREEMPT_UNLOCK;

    if (retval != NULL)
        *retval = thread_to_join->retval;

    if (thread_to_join == main_thread)
        return 0;

    VALGRIND_STACK_DEREGISTER(thread_to_join->valgrind_stack_id);
    free(thread_to_join->context.uc_stack.ss_sp);
    free(thread_to_join);
    return 0;
}

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 *
 * L'attribut noreturn aide le compilateur à optimiser le code de
 * l'application (élimination de code mort). Attention à ne pas mettre
 * cet attribut dans votre interface tant que votre thread_exit()
 * n'est pas correctement implémenté (il ne doit jamais retourner).
 */
extern void thread_exit(void *retval)
{
    PREEMPT_LOCK;
    current_thread->retval = retval;
    current_thread->state = TERMINATED;
    BRTREE_ERASE(current_thread, &threads, thread_struct);
    if (current_thread->who_is_waiting_for_me != NULL)
    {
        thread_struct *waiting_thread = current_thread->who_is_waiting_for_me;
        current_thread->who_is_waiting_for_me = NULL;
        BRTREE_INSERT(waiting_thread, &threads, thread_struct);
    }
    PREEMPT_UNLOCK;
    thread_yield();
    exit(0);
}

/* Obtenir la valeur de priorité du thread donné
 *
 * La valeur de priorité va de 0 (pas prioritaire) à 39 (très prioritaire)
 * La valeur de priorité par défaut est 20
 */
extern int thread_getpriority(thread_t thread)
{
    if (thread == NULL)
        return -1;
    return ((thread_struct *)thread)->priority;
}

/* Modifier la valeur de priorité du thread donné
 *
 * retourne 0 si l'exécution n'a levé aucune erreur
 *
 * La valeur de priorité va de 0 (pas prioritaire) à 39 (très prioritaire)
 * La valeur de priorité par défaut est 20
 */
extern int thread_setpriority(thread_t thread, int priority)
{
    if (thread == NULL)
        return -1;

    if (priority < 0 || priority > 39)
        return -2;

    ((thread_struct *)thread)->priority = priority;
    return 0;
}

int thread_mutex_init(thread_mutex_t *mutex)
{
    (void)mutex;
    mutex->owner = NULL;
    return 0;
}
int thread_mutex_destroy(thread_mutex_t *mutex)
{
    (void)mutex;
    return 0;
}
int thread_mutex_lock(thread_mutex_t *mutex)
{
    (void)mutex;
    PREEMPT_LOCK;
    if (mutex->owner == NULL) {
        mutex->owner = (thread_t) current_thread;
        PREEMPT_UNLOCK;
    } else {
        struct thread_struct *last = (struct thread_struct *)mutex->owner;
        while (last->next_in_mutex_queue != NULL) {
            if (BRTREE_KEY(last->next_in_mutex_queue) < BRTREE_KEY(current_thread)) break;
            last = last->next_in_mutex_queue;
        }
        if (last->next_in_mutex_queue != NULL) current_thread->next_in_mutex_queue = last->next_in_mutex_queue;
        last->next_in_mutex_queue = current_thread;
        BRTREE_ERASE(current_thread, &threads, thread_struct);
        PREEMPT_UNLOCK;
        thread_yield();
    }
    return 0;
}
int thread_mutex_unlock(thread_mutex_t *mutex)
{
    (void)mutex;
    PREEMPT_LOCK;
    if (mutex->owner != (thread_t)current_thread) return -1;

    if (current_thread->next_in_mutex_queue == NULL) {
        mutex->owner = NULL;
    } else {
        BRTREE_INSERT(current_thread->next_in_mutex_queue, &threads, thread_struct);
        mutex->owner = (thread_t) current_thread->next_in_mutex_queue;
        current_thread->next_in_mutex_queue = NULL;
    }
    PREEMPT_UNLOCK;
    return 0;
}
