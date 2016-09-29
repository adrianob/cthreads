#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "fila2.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: cthread  libcthread

cthread: $(SRC_DIR)/cthreads.c
	$(CC) -c $(SRC_DIR)/cthreads.c -I$(INC_DIR) -Wall && mv cthreads.o $(BIN_DIR)

libcthread: cthread
	ar crs $(LIB_DIR)/libcthread.a $(BIN_DIR)/support.o $(BIN_DIR)/cthreads.o

clean:
	rm -rf $(LIB_DIR)/*.a $(SRC_DIR)/*~ $(INC_DIR)/*~ *~ && mv $(BIN_DIR)/support.o . && rm $(BIN_DIR)/*.o && mv support.o $(BIN_DIR)
