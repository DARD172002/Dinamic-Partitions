#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "../include/share_memory.h"
#include <unistd.h>
#include "../include/global.h"

int main() {
    int size_of_memory;
    printf("Please insert number of lines of memory: ");
    scanf("%d", &size_of_memory);

    // Create shared memory segment
    int shmid = shmget(SHM_KEY, sizeof(Shared_memory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory
    Shared_memory *memory = (Shared_memory *)shmat(shmid, NULL, 0);
    if (memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize memory
    memory->size = size_of_memory;
    memory->free = size_of_memory;
    memory->number_of_proccess = 0;

    // Create semaphores
    sem_t *mem_sem = sem_open(SHARED_MEMORY, O_CREAT, 0666, 1);
    sem_t *mem_bit = sem_open(BITACORA, O_CREAT, 0666, 1);

    // Create bitacora file
    FILE *bitacora = fopen(BITACORA_FILE, "w");
    if (!bitacora) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(bitacora, "SIMULACIÓN INICIADA - Memoria: %d líneas\n", size_of_memory);
    fclose(bitacora);

    printf("Ambiente inicializado correctamente.\n");
    printf("Memoria: %d líneas\n", size_of_memory);
    printf("Semáforos y bitácora creados.\n");

    // Detach shared memory
    shmdt(memory);
    return 0;
}
