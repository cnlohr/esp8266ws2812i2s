include ../user.cfg
include ../esp82xx/common.mf

CC = gcc
CFLAGS= -std=gnu99 -Wall -Wextra -O2 -march=native $(OPTS)
LOADLIBES = -lm

wsend : wsend.o

clean :
	rm -rf wsend.o wsend *~

