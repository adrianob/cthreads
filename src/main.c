#include <stdbool.h>
#include <ucontext.h>
#include "include/support.h"
#include "include/cthread.h"
#include "include/cdata.h"


bool first_run = true;
int last_used_tid = 0;

int main(int argc, const char *argv[]) {
  return 0;
}

int ccreate (void *(*start)(void *), void *arg) {
  //TODO run tasks concerning thread management
  if(first_run){//do this for all library functions
    //TODO create main thread
    first_run = false;
  }

  TCB_t *thread;
  ucontext_t *current_context, function_context:
  char function_stack[SIGSTKSZ];
  

  thread->tid = ++last_used_tid;
  thread->state = PROCST_CRIACAO;
  thread->ticket = Random2();
  thread->context = getcontext(current_context);


  //TODO make this work
  makecontext(&function_context, start,
        arg, &function_context, thread->context, &i_from_iterator);

  thread->state = PROCST_APTO;
  //TODO insert thread into apt list
  return -1;
}
