include ../user.cfg
include ../esp82xx/common.mf

CC = gcc
CFLAGS= -std=gnu99 -Wall -Wextra -O2 -march=native $(OPTS) -DNO_CTYPES
LOADLIBES = -lm

wsend : wsend.o ../user/pattern.o

clean :
	rm -rf wsend.o wsend *~

