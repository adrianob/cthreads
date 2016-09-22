#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata"

#define OK    0
#define ERRO -1

FILA2 FilaSemaforo;

int csem_init(csem_t *sem, int count)
{
  int init = CreateFila2(&FilaSemaforo);
  if(init != 0)
    return ERRO;

  csem_t *sem = (csem_t*)malloc(sizeof(csem_t));
  sem->count = 1;
  sem->fila = *FilaSemaforo;

  return OK;
}

/*cwait(csem_t *sem)
{
  if(sem->count <=0){

  }

}*/
