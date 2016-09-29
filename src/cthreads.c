#include <ucontext.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


#define stackSize SIGSTKSZ

int first_run = TRUE;
int executing = TRUE;
int last_used_tid = 0;
TCB_t current_thread;//thread currently executing
TCB_t main_thread;
ucontext_t finish_context;
FILA2 ready_list;
FILA2 blocked_list;
FILA2 blocked_ids;


void * finish_thread(void) {
  //remove from blocked list if same tid
  TCB_t *iter_thread = (TCB_t *) malloc(sizeof(TCB_t));
  int * iter_id;

  FirstFila2(&blocked_ids);

  int iterations = 0;
 //remove id from list
  do {
    iter_id = ((int *)GetAtIteratorFila2(&blocked_ids));
    if( iter_id != NULL){
      if(*iter_id == current_thread.tid){
        DeleteAtIteratorFila2(&blocked_ids);
        break;
      }
    }
    iterations++;
  } while( NextFila2(&blocked_ids) == 0);

  FirstFila2(&blocked_list);
  int i;
  int end;
  for(i = 0; i < iterations; i++){
    iter_thread = ((TCB_t *)GetAtIteratorFila2(&blocked_list));
    if(iter_thread != NULL ){
        DeleteAtIteratorFila2(&blocked_list);
        iter_thread->state = PROCST_APTO;
        AppendFila2(&ready_list, iter_thread);
        break;
    }
    end = NextFila2(&blocked_list);
  }

  executing = FALSE;

 scheduler();
  return NULL;
}

void initialize(void){
  CreateFila2(&ready_list);
  CreateFila2(&blocked_list);
  CreateFila2(&blocked_ids);

  main_thread.tid = 0;
  main_thread.state = PROCST_APTO;
  main_thread.ticket = Random256();
  first_run = FALSE;

  getcontext(&main_thread.context);

  current_thread = main_thread;
  getcontext(&finish_context);

  finish_context.uc_link          = NULL;
  finish_context.uc_stack.ss_sp   = (char*) malloc(stackSize);
  finish_context.uc_stack.ss_size = stackSize;

  makecontext(&finish_context, (void (*)(void)) finish_thread, 0); 
}

int ccreate (void *(*start)(void *), void *arg) {
  if(first_run){ initialize(); }
  TCB_t *thread = (TCB_t *) malloc(sizeof(TCB_t));
  ucontext_t * context = malloc(sizeof(ucontext_t));

  thread->tid = ++last_used_tid;
  thread->state = PROCST_CRIACAO;
  thread->ticket = Random256();

  getcontext(context);

  context->uc_stack.ss_sp   = (char*) malloc(stackSize);
  context->uc_stack.ss_size = stackSize;
  context->uc_link          = &finish_context;

  makecontext(context, (void (*)(void))start, 1, arg);

  thread->state = PROCST_APTO;
  thread->context = *context;
  AppendFila2(&ready_list, thread);
  return thread->tid;
}

void addToBlockedList(int tid){
  int *id = malloc(sizeof(int));
  *id = tid;
  AppendFila2(&blocked_ids, (void*)id);
  TCB_t * blocked_thread = (TCB_t *)malloc(sizeof(TCB_t));
  *blocked_thread = current_thread;
  AppendFila2(&blocked_list, (void *)blocked_thread);
}

int cjoin(int tid){
  if(first_run){ initialize(); }
 addToBlockedList(tid);

  executing = FALSE;

  scheduler();

  return 0;
}

unsigned int Random256(void){
  unsigned int rand = Random2();
  float avg = (float)rand/65535;
  rand = (int) (avg*255);
  return rand;
}
void scheduler(void){
  if (!executing) {//CPU is free, execute next thread in ready list
    unsigned int new_ticket = Random256();
  TCB_t * thread;
   int closest = 64000;

    FirstFila2(&ready_list);
  do {
    thread = (TCB_t *)GetAtIteratorFila2(&ready_list);
    if(thread != NULL && abs(thread->ticket - new_ticket) < closest){
      closest = abs(thread->ticket - new_ticket);
      current_thread = *thread;
    }
  } while( NextFila2(&ready_list) == 0);


  FirstFila2(&ready_list);
  do {
    thread = ((TCB_t *)GetAtIteratorFila2(&ready_list));
    if(thread != NULL && thread->ticket == current_thread.ticket){
	    DeleteAtIteratorFila2(&ready_list);
    }
  } while( NextFila2(&ready_list) == 0);

    current_thread.state = PROCST_EXEC;
   executing = TRUE;
  setcontext(&current_thread.context);
  }
}

int cyield(void){
  if(first_run){ initialize(); }
  TCB_t *thread = (TCB_t *) malloc(sizeof(TCB_t));
  *thread = current_thread;
  thread->state = PROCST_APTO;
  AppendFila2(&ready_list, thread);
  executing = FALSE;
  scheduler();
  return 0;
}

int cidentify(char *name, int size){
   char *str = "Adriano Carniel Benin\t\t Numero = 00173464 \nGabriel Alexandre Zillmer\t Numero = 00243683 \nLucas Valandro da Rocha\t\t Numero = 00243675";
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
  TCB_t thread = current_thread;
  sem->count = sem->count - 1;
  if(sem->count > 0){ //CPU is free them we associate a thread using a ticket number.
    if(FirstFila2(&ready_list) == 0){ //!!
      executing = FALSE;
      scheduler();
      executing = TRUE;
    }
    else return ERRO;
  }
  else{ //The thread needs to be blocked.
    (&thread)->state = PROCST_BLOQ;
    if(AppendFila2(sem->fila, &thread) == 0){
      if(FirstFila2(&ready_list) == 0){
        executing = FALSE;
        scheduler();
        executing = TRUE;
      }
      else return ERRO;
    }
    else return ERRO;
  }
  return OK;
}

int csignal(csem_t *sem){
  TCB_t thread = current_thread;
  sem->count += 1;
  if(sem->count > 0){ //CPU is free,blocked_list->first needs to pass to the ready_list
    if(FirstFila2(sem->fila) == 0){
      thread = *((TCB_t *)GetAtIteratorFila2(sem->fila));
      if(AppendFila2(&ready_list, &thread) == 0)
        return OK;
      else return ERRO;
    }
    else return ERRO;
  }
  return OK;
}


