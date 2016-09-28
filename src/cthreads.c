#include <ucontext.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#inculde <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

void initialize(void);
void * finish_thread(void);
void scheduler(void);

first_run = TRUE;
executing = TRUE;
int last_used_tid = 0;
TCB_t current_thread;//thread currently executing
ucontext_t * finish_context;
FILA2 ready_list;
FILA2 blocked_list;
FILA2 blocked_ids;

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
}

int main(int argc, const char *argv[]) {
	int	id0, id1;
	int i;

	id0 = ccreate(func0, (void *)&i);
	id1 = ccreate(func1, (void *)&i);

	printf("Eu sou a main após a criação de ID0 e ID1\n");

	cjoin(id0);
	cjoin(id1);

	printf("Eu sou a main voltando para terminar o programa\n");

  return 0;
}

void * finish_thread(void) {
  //remove from blocked list if same tid
  TCB_t *iter_thread;
  int * iter_id;
  FirstFila2(&blocked_list);
  FirstFila2(&blocked_ids);
  do {
    iter_id = ((int *)GetAtIteratorFila2(&blocked_ids));
    iter_thread = ((TCB_t *)GetAtIteratorFila2(&blocked_list));
    if(GetAtIteratorFila2(&blocked_list) != NULL && GetAtIteratorFila2(&blocked_ids) != NULL){
      if(*iter_id == current_thread.tid){
        DeleteAtIteratorFila2(&blocked_list);
        DeleteAtIteratorFila2(&blocked_ids);
        iter_thread->state = PROCST_APTO;
        AppendFila2(&ready_list, iter_thread);
        break;
      }
    }
  } while(NextFila2(&blocked_list) == 0 && NextFila2(&blocked_ids) == 0);
	executing = false;
	scheduler();
  return NULL;
}

void initialize(void){
  char function_stack[SIGSTKSZ];
	finish_context = (ucontext_t*) malloc (sizeof(ucontext_t));
	getcontext(finish_context);

  finish_context->uc_stack.ss_sp   = function_stack;
  finish_context->uc_stack.ss_size = sizeof(function_stack);
	finish_context->uc_link = NULL;
	makecontext(finish_context, (void (*)(void))finish_thread, 0);

  CreateFila2(&ready_list);
  CreateFila2(&blocked_list);
  CreateFila2(&blocked_ids);
  TCB_t * main_thread = (TCB_t *)malloc(sizeof(TCB_t));

  main_thread->tid = 0;
  main_thread->state = PROCST_APTO;
  main_thread->ticket = Random2();
  first_run = false;
  getcontext(&(main_thread->context));
  current_thread = *main_thread;
}

int ccreate (void *(*start)(void *), void *arg) {
  if(first_run){ initialize(); }
  TCB_t *thread = (TCB_t *) malloc(sizeof(TCB_t));
  ucontext_t * context = malloc(sizeof(ucontext_t));

  char function_stack[SIGSTKSZ];

  thread->tid = ++last_used_tid;
  thread->state = PROCST_CRIACAO;
  thread->ticket = Random2();

  getcontext(context);

  context->uc_stack.ss_sp   = function_stack;
  context->uc_stack.ss_size = sizeof(function_stack);
  context->uc_link          = finish_context;

  makecontext(context, (void (*)(void))start, 1, arg);

  thread->state = PROCST_APTO;
  thread->context = *context;
  AppendFila2(&ready_list, thread);

  return thread->tid;
}

int cjoin(int tid){
  if(first_run){ initialize(); }
  int *id = malloc(sizeof(int));
  *id = tid;
  FirstFila2(&blocked_list);
  AppendFila2(&blocked_list, &current_thread);
  FirstFila2(&blocked_ids);
  AppendFila2(&blocked_ids, id);
  executing = false;

  scheduler();
  return 0;
}

void scheduler(void){
  if (!executing) {//CPU is free, execute next thread in ready list
    //unsigned int new_ticket = Random2();
    TCB_t *chosen_thread = NULL;

    //TODO run thread with tid closest to new_ticket
    FirstFila2(&ready_list);
    chosen_thread = (TCB_t *)GetAtIteratorFila2(&ready_list);
    DeleteAtIteratorFila2(&ready_list);

    chosen_thread->state = PROCST_EXEC;
    current_thread = *chosen_thread;

   executing = true;
    setcontext(&(chosen_thread->context));
  }
}

int cyield(void){
  if(first_run){ initialize(); }
  scheduler();
  return 0;
}

int cidentify(char *name, int size){
   char *str = "Adriano Carniel Benin\t\t Numero = 00 \nGabriel Alexandre Zillmer\t Numero = 00243683 \nLucas Valandro da Rocha\t\t Numero = 00243675";
   strncpy(name,str,size - 1);
   if(strcmp(name,str) == 0)
    return OK;
   else return ERRO;
}

int csem_init(csem_t *sem, int count)
{
  FILA2 *block_list = NULL; //Each (csem_t *sem) has one block_list
  int init = CreateFila2(block_list);
  if(init != 0)
    return ERRO;
  sem = (csem_t*)malloc(sizeof(csem_t));
  sem->count = count;
  sem->fila = block_list;

  return OK;
}

int cwait(csem_t *sem){
  TCB_t thread = *current_thread;

  sem->count = sem->count - 1;
  if(sem->count > 0){ //CPU is free them we associate a thread using a ticket number.
    if(FirstFila2(&ready_list) == 0){ //!!
      executing = FALSE;
      update_threads();
      executing = TRUE;
    }
    else return ERRO;
  }
  else{ //The thread needs to be blocked.
    (&thread)->state = PROCST_BLOQ;
    if(AppendFila2(sem->fila, &thread) == 0){
      if(FirstFila2(&ready_list) == 0){
        executing = FALSE;
        update_threads();
        executing = TRUE;
      }
      else return ERRO;
    }
    else return ERRO;
  }
  return OK;
}


int csignal(csem_t *sem){
  TCB_t thread = *current_thread;

  sem->count += 1;
  if(sem->count > 0){ //CPU is free, so the first of the blocked_list needs to pass to the ready_list
    if(FirstFila2(sem->fila) == 0){));
      thread = *((TCB_t *)GetAtIteratorFila2(sem->list));
      if(AppendFila2(&ready_list, &thread) == 0)
        return OK;
      else return ERRO;
    }
    else return ERRO;
  }
  return OK;
}
