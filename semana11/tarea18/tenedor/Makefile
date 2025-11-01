# =====================================================
# Makefile para compilar programas de prueba con Fork y Exec
# =====================================================

# Compilador (puedes cambiarlo a nachos-gcc si estás en NachOS)
CC = gcc

# Flags de compilación (pueden ser distintos en NachOS)
CFLAGS = -Wall -Wextra -g

# Archivos fuente
SRC = fork.c Fork.c ForkExec.c Memory.c SimpleFork.c

# Archivos ejecutables (se generan sin la extensión .c)
PROGS = $(SRC:.c=)

# Regla por defecto: compilar todos
all: $(PROGS)

# Regla genérica para compilar cada .c a ejecutable
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Limpieza de archivos compilados
clean:
	rm -f $(PROGS) *.o

# Ejecución rápida de cada programa (por conveniencia)
run-fork:
	./fork

run-Fork:
	./Fork

run-ForkExec:
	./ForkExec

run-Memory:
	./Memory

run-SimpleFork:
	./SimpleFork