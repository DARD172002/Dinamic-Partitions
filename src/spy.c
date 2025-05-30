#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "../include/share_memory.h"
#include "../include/global.h"

sem_t *mem_sem;

int main() {
    int shmid = shmget(SHM_KEY, sizeof(Shared_memory), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    Shared_memory *memory = (Shared_memory *) shmat(shmid, NULL, 0);
    if (memory == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    mem_sem = sem_open(SHARED_MEMORY, 0);
    if (mem_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_wait(mem_sem);

    printf("\n===== ESTADO DE LA MEMORIA =====\n");
    char simbolos[100];
    for (int i = 0; i < 100; i++) {
        if (i < 26)
            simbolos[i] = 'A' + i;
        else if (i < 52)
            simbolos[i] = 'a' + (i - 26);
        else
            simbolos[i] = '*';
    }

    char memoria[memory->size];
    for (int i = 0; i < memory->size; i++) memoria[i] = '-';

    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            for (int j = memory->process[i].init;
                 j < memory->process[i].init + memory->process[i].size; j++) {
                if (j >= 0 && j < memory->size)
                    memoria[j] = simbolos[i];
            }
        }
    }

    printf("[");
    for (int i = 0; i < memory->size; i++) {
        printf("%c", memoria[i]);
    }
    printf("]\n");

    printf("Procesos:\n");
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            printf("  %c = PID %d (%d líneas desde %d)\n",
                   simbolos[i],
                   memory->process[i].pid,
                   memory->process[i].size,
                   memory->process[i].init);
        }
    }

    printf("\n===== ESTADO DE LOS PROCESOS =====\n");

    printf("Procesos activos en memoria:\n");
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            printf("  PID %d - Ocupa %d líneas desde %d\n",
                   memory->process[i].pid,
                   memory->process[i].size,
                   memory->process[i].init);
        }
    }

    sem_post(mem_sem);

    shmdt(memory);
    return 0;
}
