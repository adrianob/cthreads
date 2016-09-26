#include <stdbool.h>
#include <ucontext.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#inculde <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


bool first_run = true;
bool executing = true;
int last_used_tid = 0;
TCB_t current_thread;//thread currently executing
FILA2 ready_list;


void * printInt(void *length){
  printf("funcao %d\n", 5);
  /*
    int i;
    for(i = 0; i<*((int *)length);i++)
    printf("%d\n", i);
  */
  return;
}

int main(int argc, const char *argv[]) {
  int x;
  int pid = ccreate(printInt, (void *)&x);
  printf("pid1 %d\n", pid);
  cjoin(pid);
  printf("exiting main\n");
  return OK;
}

int ccreate (void *(*start)(void *), void *arg) {
  if(first_run){ update_threads(); }
  TCB_t thread;
  ucontext_t context, main_context;

  char function_stack[SIGSTKSZ];

  thread.tid = ++last_used_tid;
  thread.state = PROCST_CRIACAO;
  thread.ticket = Random2();

  getcontext(&context);

  context.uc_link          = &main_context;
  context.uc_stack.ss_sp   = function_stack;
  context.uc_stack.ss_size = sizeof(function_stack);

  makecontext(&context, (void (*)(void))start, 1, arg);

  thread.state = PROCST_APTO;
  thread.context = context;
  AppendFila2(&ready_list, &thread);

  getcontext(&main_context);
  return thread.tid;
}

int cjoin(int tid){
  FirstFila2(&ready_list);
  AppendFila2(&ready_list, &current_thread);
  executing = false;

  TCB_t next_thread, iter_thread;

  FirstFila2(&ready_list);
  do {
    iter_thread = *((TCB_t *)GetAtIteratorFila2(&ready_list));
    if(GetAtIteratorFila2(&ready_list) != NULL){
      if(iter_thread.tid == tid){
        next_thread = iter_thread;
	break;
      }}
  } while(NextFila2(&ready_list) == 0);

  DeleteAtIteratorFila2(&ready_list);
  next_thread.state = PROCST_EXEC;
  current_thread = next_thread;
  executing = true;
  setcontext(&(current_thread.context));
  return OK;
}

void update_threads(void){
  if(first_run){
    CreateFila2(&ready_list);
    TCB_t main_thread;

    main_thread.tid = 0;
    main_thread.state = PROCST_APTO;
    main_thread.ticket = Random2();
    first_run = false;
    getcontext(&(main_thread.context));
    current_thread = main_thread;
  }

  if (!executing) {//CPU is free, execute next thread in ready list
    unsigned int new_ticket = Random2();
    TCB_t *next_thread = NULL;

    FirstFila2(&ready_list);
    next_thread = GetAtIteratorFila2(&ready_list);
    DeleteAtIteratorFila2(&ready_list);

    //TODO run thread with tid closest to new_ticket
    next_thread->state = PROCST_EXEC;
    current_thread = *next_thread;
    setcontext(&(current_thread.context));
  }
}

/*
int cyield(void){
  TCB_t thread = *current_thread;
  (&thread)->state = PROCST_APTO;
  AppendFila2(&ready_list, &thread);
  current_thread = NULL;

  update_threads();
  return OK;
}
*/

int cidentify(char *name, int size){
   char *str = "Adriano Carniel Benin\t\t Numero = 00 \nGabriel Alexandre Zillmer\t Numero = 00243683 \nLucas Valandro da Rocha\t\t Numero = 00243675";
   strncpy(name,str,size - 1);
   if(strcmp(name,str) == 0)
    return OK;
   else return ERRO;
}
/*

int csem_init(csem_t *sem, int count)
{
  FILA2 block_list; //Each (csem_t *sem) has one block_list
  int init = CreateFila2(&block_list);
  if(init != 0)
    return ERRO;
  sem = (csem_t*)malloc(sizeof(csem_t));
  sem->count = count;
  sem->fila = *block_list;

  return OK;
}
*/

/*
int cwait(csem_t *sem){
  TCB_t thread = *current_thread;
  sem->count = sem->count - 1;
  if(sem->count > 0){ //CPU is free them we associate a thread using a ticket number.
    //raffling_ticket(&ready_list);

  }
  else{ //Them the thread needs to be blocked.
    (&thread)->state = PROCST_BLOQ;
    AppendFila2(&sem->fila, &current_thread);
    current_thread = NULL;
  }
}
*/
/*
int csignal(csem_t *sem){
  TCB_t thread;

  sem->count = count++;

  if(sem->count > 0){ //CPU is free, so the first of the blocked_list needs to pass to the ready_list
    if(FirstFila2(&sem->fila) == 0){
      thread = *((TCB_t *)GetAtIteratorFila2(&sem->list));
      if(AppendFila2(&ready_list, &thread) == 0)
        return OK;
      else return ERRO;
    }
    else return ERRO;
  }
  return OK;
}
*/
