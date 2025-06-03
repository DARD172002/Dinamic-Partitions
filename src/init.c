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
#include <time.h>

// Función para registrar en la bitácora (usa el mismo semáforo)
void log_action(sem_t *mem_sem, const char* action, int pid, const char* type, int lines) {
    FILE *bitacora = fopen(BITACORA_FILE, "a");
    if (!bitacora) {
        perror("fopen");
        return;
    }
    
    time_t now = time(NULL);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(bitacora, "[%s] PID %d: %s (%s) - Líneas: %d\n", 
            time_str, pid, action, type, lines);
    
    fclose(bitacora);
}


int main() {
    int size_of_memory;
    printf("Please insert number of lines of memory: ");
    scanf("%d", &size_of_memory);
    if (size_of_memory <= 0) {
    fprintf(stderr, "Name of line invalidate,the number must be greater than cero.\n");
    exit(EXIT_FAILURE);
}

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

    for (int i = 0; i < 100; i++) {
    memory->process[i].pid = -1;
    memory->process[i].init = -1;
    memory->process[i].size = 0;
    memory->process[i].time = 0;
}


    // Create semaphores
    sem_t *mem_sem = sem_open(SHARED_MEMORY, O_CREAT, 0666, 1);
    //sem_t *mem_bit = sem_open(BITACORA, O_CREAT, 0666, 1);
    if (mem_sem == SEM_FAILED) {
    perror("sem_open mem_sem");
    exit(EXIT_FAILURE);
}

/*if (mem_bit == SEM_FAILED) {
    perror("sem_open mem_bit");
    exit(EXIT_FAILURE);
}*/
    // Create bitacora file
        // Inicializar bitácora (usando el semáforo)
    sem_wait(mem_sem);
    FILE *bitacora = fopen(BITACORA_FILE, "w");
    if (!bitacora) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    //register of begin simulation
    time_t now = time(NULL);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(bitacora, "SIMULACIÓN INICIADA - Memoria: %d líneas\n", size_of_memory);
    fprintf(bitacora, "Fecha/Hora: %s\n", time_str);
    fprintf(bitacora, "================================\n\n");
    fclose(bitacora);
    // Registrar acción en bitácora
    // Registrar acción inicial
    log_action(mem_sem, "Inicialización", getpid(), "sistema", size_of_memory);
    sem_post(mem_sem);

    printf("Ambiente inicializado correctamente.\n");
    printf("Memoria: %d líneas\n", size_of_memory);
    printf("Semáforos y bitácora creados.\n");

    // Detach shared memory
    shmdt(memory);
    return 0;
}
