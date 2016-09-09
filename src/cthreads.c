#include <stdbool.h>
#include <ucontext.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


bool first_run = true;
int last_used_tid = 0;
PFILA2 ready_list = NULL;
TCB_t *current_thread = NULL;//thread currently executing

int main(int argc, const char *argv[]) {
  CreateFila2(ready_list);
  return 0;
}

int ccreate (void *(*start)(void *), void *arg) {
  TCB_t * thread;
  ucontext_t context, main_context;

  char function_stack[SIGSTKSZ];

  thread->tid = ++last_used_tid;
  thread->state = PROCST_CRIACAO;
  thread->ticket = Random2();

  getcontext(&context);

  context.uc_link          = &main_context;
  context.uc_stack.ss_sp   = function_stack;
  context.uc_stack.ss_size = sizeof(function_stack);

  makecontext(&context, start, 1, arg);

  thread->state = PROCST_APTO;
  thread->context = context;
  AppendFila2(ready_list, thread);

  getcontext(&main_context);
  update_threads();
  return -1;
}

int cyield(void){
  TCB_t thread = *current_thread;
  (&thread)->state = PROCST_APTO;
  AppendFila2(ready_list, &thread);
  current_thread = NULL;

  update_threads();
  return -1;
}

int cjoin(int tid){
  return -1;
}

void update_threads(void){
  if(first_run){//do this for all library functions
    TCB_t * main_thread;

    main_thread->tid = 0;
    main_thread->state = PROCST_CRIACAO;
    main_thread->ticket = Random2();
    getcontext(&main_thread->context);

    AppendFila2(ready_list, &main_thread);
    first_run = false;
  }

  if (!current_thread) {//CPU is free
    unsigned int new_ticket = Random2();
    TCB_t *next_thread = NULL;

    FirstFila2(ready_list);
    next_thread = GetAtIteratorFila2(ready_list);
    DeleteAtIteratorFila2(ready_list);

    //TODO run thread with tid closest to new_ticket
    next_thread->state = PROCST_EXEC;
    current_thread = next_thread;
    setcontext(&(next_thread->context));
  }
}
