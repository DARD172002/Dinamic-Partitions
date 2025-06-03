# Compilador y flags
CC = gcc
CFLAGS = -g -Wall

# Archivos fuente y objeto
INIT = src/init.c
endProcess = src/endProcess.c
PRODUCTOR = src/productor.c
SPY = src/spy.c
OBJ = init.o endProcess.o

# Regla por defecto
all: init endProcess productor spy

# Compilación
init: $(SRC) include/global.h include/share_memory.h $(INIT)
	$(CC) $(CFLAGS) $(INIT) -o init
productor: $(SRC) include/global.h include/share_memory.h $(PRODUCTOR)
	$(CC) $(CFLAGS) $(PRODUCTOR) -o productor
spy:include/global.h include/share_memory.h $(SPY)
	$(CC) $(CFLAGS) $(SPY) -o spy
endProcess:include/global.h include/share_memory.h $(endProcess)
	$(CC) $(CFLAGS) $(endProcess) -o endProcess



# Limpieza de ejecutables y objetos
clean:

	rm -f

# Alias para limpieza rápida del ejecutable
delete: clean
