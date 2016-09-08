#include <stdbool.h>
#include <ucontext.h>
#include <stdlib.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


bool first_run = true;
int last_used_tid = 0;
PFILA2 * apt_list = NULL;
TCB_t *current_thread = NULL;//thread currently executing

int main(int argc, const char *argv[]) {
  CreateFila2(*apt_list);
  return 0;
}

int ccreate (void *(*start)(void *), void *arg) {
  TCB_t *thread;
  ucontext_t *current_context;
  ucontext_t function_context;
  char function_stack[SIGSTKSZ];

  thread->tid = ++last_used_tid;
  thread->state = PROCST_CRIACAO;
  thread->ticket = Random2();
  getcontext(&(thread->context));

  //TODO make this work
  /*makecontext(&function_context, start,*/
        /*arg, &function_context, thread->context, &i_from_iterator);*/

  thread->state = PROCST_APTO;
  AppendFila2(*apt_list, thread);
  //update_threads();
  if(first_run){//do this for all library functions
    //TODO create main thread
    //current_thread = main_thread;
    first_run = false;
  }
  return -1;
}

int cyield(void){
  TCB_t thread = *current_thread;
  (&thread)->state = PROCST_APTO;
  AppendFila2(*apt_list, &thread);
  current_thread = NULL;

  //update_threads();
  if(first_run){//do this for all library functions
    //TODO create main thread
    first_run = false;
  }
  return -1;
}

void update_threads(void){
  if (!current_thread) {//CPU is free
    int new_ticket = Random2();
    TCB_t *next_thread = NULL;
    //run thread with tid closest to new_ticket
    //setcontext
    current_thread = next_thread;
  }
}
