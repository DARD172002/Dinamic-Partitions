# Compilador y flags
CC = gcc
CFLAGS = -g -Wall

# Archivos fuente y objeto
SRC = src/init.c
OBJ = init.o
EXEC = init

# Regla por defecto
all: $(EXEC)

# Compilación
$(EXEC): $(SRC) include/global.h include/share_memory.h
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC)

# Limpieza de ejecutables y objetos
clean:
	rm -f $(EXEC) $(OBJ)

# Alias para limpieza rápida del ejecutable
delete: clean
