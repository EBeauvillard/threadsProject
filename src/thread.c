#include "../headers/thread.h"

#include <sys/ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include <stdlib.h>
#include<sys/queue.h>
#include <ucontext.h>
#include <valgrind/valgrind.h>
#include <stdio.h>
#include <execinfo.h>
#include <string.h>

#include <pthread.h>
#include <signal.h>

// Structure d'un thread
struct thread {
    // id du thread
    int id;
    // Attribut pour savoir si le thread est encore en cours
    int is_running;
    // Valeur de retour du thread
    void* retval;
    // Contexte du thread
    ucontext_t thread_context;
    // Attribut spécial utilisé par QueueBSD pour stocker la position dans la liste
    STAILQ_ENTRY(thread) entry_in_list;
    // Stocke l'information sur le stack qu'on a donné à Valgrind
    int vg_stack;
    // Pointeur vers un autre thread qui attend la fin de ce thread
    struct thread * awaiting_thread;
    // Pointeur vers un autre thread qui attend le unlock du mutex
    struct thread * mutex_awaiting_thread;
    // Attribut qui vaut 1 si on est dans l'attente d'un autre thread (join)
    int waiting_another_thread;
};


// Initialisation de la file
STAILQ_HEAD(head_list, thread) thread_list = STAILQ_HEAD_INITIALIZER( thread_list );
// Variable globale permettant de gerer la taille de la file
static int size = 0;
// Booleen pour verifier si le thread du main a ete initialise
static int init = 1;
// Contexte de la fonction main, qui sera utilise a la fin de l'execution
ucontext_t context_main;


// Liberation du thread
void free_thread(struct thread* t) {
  if(t->vg_stack != -1){
    VALGRIND_STACK_DEREGISTER(t->vg_stack);
    free(t->thread_context.uc_stack.ss_sp);
  }
  free(t);
}


// Renvoie un pointeur vers le thread actuel
extern thread_t thread_self(void){
  struct thread * current = STAILQ_FIRST(&thread_list);

  if (init) {
    getcontext(&context_main);
    init = 0;
    thread_create((void**)&current, NULL, NULL);
  }

  return (thread_t) current;
}


// Lance la fonction du thread
void thread_run(void* (*func)(void*), void* funcarg) {
  if ((func != NULL))
    thread_exit(func(funcarg));

  return;
}


// Cree un nouveau thread qui executera func avec les arguments funcarg
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
  if (init) {
    getcontext(&context_main);
    init = 0;
    thread_t current = NULL;
    thread_create(&current, NULL, NULL);
  }

  struct thread * created_thread = malloc(sizeof(struct thread));
  created_thread->id = size++;
  created_thread->is_running = 1;
  created_thread->vg_stack = -1;
  created_thread->awaiting_thread = NULL;
  created_thread->mutex_awaiting_thread = NULL;
  created_thread->waiting_another_thread = 0;


  if (getcontext(&(created_thread->thread_context)) == -1)
    return -1;

  // Si ce n'est pas le thread de main, on cree un nouveau stack
  if(size > 1){
    created_thread->thread_context.uc_stack.ss_size = 64*1024;
    created_thread->thread_context.uc_stack.ss_sp = malloc(created_thread->thread_context.uc_stack.ss_size);

    // On informe Valgrind du fait qu'on cree un stack
    created_thread->vg_stack = VALGRIND_STACK_REGISTER(created_thread->thread_context.uc_stack.ss_sp,
                  created_thread->thread_context.uc_stack.ss_sp + created_thread->thread_context.uc_stack.ss_size);

    makecontext(&(created_thread->thread_context), (void (*) (void)) thread_run, 2, func, funcarg);
  }

  *newthread = created_thread;

  struct thread * current_head = STAILQ_FIRST(&thread_list);
  STAILQ_INSERT_HEAD(&thread_list, created_thread, entry_in_list);

  // Si ce n'est pas la fonction main, on change de contexte vers la nouvelle fonction creee
  if(size > 1){
    swapcontext(&(current_head->thread_context), &(created_thread->thread_context));
  }

  return 0;
}


// Passe la main a un autre thread
extern int thread_yield(void){
  struct thread * previous = (struct thread *) thread_self();

  if (previous == NULL)
    return -1;

  STAILQ_REMOVE_HEAD(&thread_list, entry_in_list);
  struct thread* current = STAILQ_FIRST(&thread_list);
  // On ne remet pas dans la file un thread qui est en attende d'un autre
  if ( !previous->waiting_another_thread )
    STAILQ_INSERT_TAIL(&thread_list, previous, entry_in_list);

  if (current != NULL) {
    swapcontext(&(previous->thread_context), &(current->thread_context));
  }

  return 0;
}


/* Attend la fin d'exécution d'un thread.
 * la valeur renvoyee par le thread est placee dans *retval.
 * si retval est NULL, la valeur de retour est ignoree.
 */
extern int thread_join(thread_t thread, void **retval){
  struct thread* joined = (struct thread*) thread;

  if(joined->is_running){
    joined->awaiting_thread = thread_self();
    joined->awaiting_thread->waiting_another_thread = 1;
    thread_yield();
  }

  if (retval != NULL){
      *retval = joined->retval;
  }

  free_thread(joined);
  return 0;
}


/* Termine le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 */
extern void __attribute__ ((__noreturn__)) thread_exit(void *retval){

  struct thread * current = STAILQ_FIRST(&thread_list);
  STAILQ_REMOVE_HEAD(&thread_list, entry_in_list);
  size--;

  if(current->awaiting_thread != NULL)
    STAILQ_INSERT_HEAD(&thread_list, current->awaiting_thread, entry_in_list);

  struct thread * next_thread = STAILQ_FIRST(&thread_list);

  current->retval = retval;

  current->is_running = 0;

  // Si c'est la fonction main qui termine, on sauvegarde son contexte et on change
  if ( next_thread && (current-> vg_stack == -1)){
      swapcontext(&context_main, &next_thread->thread_context);
      // Important : sinon le programme ne quitte jamais
      exit (0);
  }
  else if ( next_thread ){
      setcontext( &next_thread->thread_context );
  }

  // L'execution est terminee, on remet current dans la queue pour qu'il soit free par le destructor
  STAILQ_INSERT_HEAD(&thread_list, current, entry_in_list);
  // Puis on change vers le contexte de main
  setcontext(&context_main);
  exit(0);
}


// Fonction appellee a la fin de la lib, qui va free le thread du main
__attribute__ ((destructor))
extern void destruct (void) {
  struct thread * current_thread = STAILQ_FIRST(&thread_list);

  if(current_thread)
    free_thread(current_thread);
}


// Initialisation de la structure du mutex
int thread_mutex_init(thread_mutex_t *mutex) {
  if (mutex == NULL)
    return -1;
  
  mutex->locked = 0;
  mutex->current_locking_thread = NULL;
  
  return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
  // Rien de particulier à faire
  return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
  struct thread * current = thread_self();

  // Si le mutex est verrouille
  if(mutex->locked == 1){
    // Recuperation du thread qui verrouille
    struct thread * locking = mutex->current_locking_thread;

    // Il faut informer le thread qui bloque qu'on attend le deverrouillage
    int ok = 0;
    do {
      // Si aucun thread n'est en attente on se stocke directement
      // dans celui qui est dans le section critique
      if(locking->mutex_awaiting_thread == NULL){
        locking->mutex_awaiting_thread = current;
        ok = 1;
      }
      // Sinon on va chercher le premier thread qui ne stocke
      // pas un pointeur vers un autre thread
      else{
        locking = locking->mutex_awaiting_thread;
      }
    }while(ok == 0);

    // On sauvegarde qu'on attend la fin d'une execution pour sortir de la queue
    current->waiting_another_thread = 1;
    thread_yield();
  }

  // Mise a jour du pointeur vers le thread actuel et verrouillage
  mutex->current_locking_thread = current;
  mutex->locked = 1;

  return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
  struct thread * current = thread_self();

  // Si un thread etait en attente du mutex on le remet dans la queue
  int waiting_thread = 0;
  if(current->mutex_awaiting_thread != NULL){
    // On sort le thread actuel du début de la file
    struct thread * current_head = STAILQ_FIRST(&thread_list);
    STAILQ_REMOVE_HEAD(&thread_list, entry_in_list);

    // On met le thread qui doit être le suivant dans la file
    current->mutex_awaiting_thread->waiting_another_thread = 0;
    STAILQ_INSERT_HEAD(&thread_list, current->mutex_awaiting_thread, entry_in_list);
    current->mutex_awaiting_thread = NULL;

    // On remet le thread actuel en tête, pour mettre l'autre thread en 2eme position
    STAILQ_INSERT_HEAD(&thread_list, current_head, entry_in_list);

    waiting_thread = 1;
  }
  
  // On deverrouille le mutex
  mutex->current_locking_thread = NULL;
  mutex->locked = 0;
  
  // Si on vient de libérer le mutex on yield pour éviter de le reprendre immédiatement
  if(waiting_thread)
    thread_yield();

  return 0;
}
