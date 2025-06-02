# Compilador y flags
CC = gcc
CFLAGS = -g -Wall

# Archivos fuente y objeto
INIT = src/init.c
endProcess = src/endProcess.c
OBJ = init.o endProcess.o

# Regla por defecto
all: init endProcess

# Compilación
init: $(SRC) include/global.h include/share_memory.h $(INIT)
	$(CC) $(CFLAGS) $(INIT) -o init


endProcess:include/global.h include/share_memory.h $(endProcess)
	$(CC) $(CFLAGS) $(endProcess) -o endProcess
# Limpieza de ejecutables y objetos
clean:

	rm -f $(EXEC) $(endProcess)   $(OBJ)

# Alias para limpieza rápida del ejecutable
delete: clean
