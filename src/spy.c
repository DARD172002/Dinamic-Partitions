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

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

const char *colores[] = {
    RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,
    "\033[91m", "\033[92m", "\033[93m", "\033[94m",
    "\033[95m", "\033[96m", "\033[97m"
};

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
    
    char memoria[memory->size];
    for (int i = 0; i < memory->size; i++) {
        memoria[i] = '-';
    }
    
    char simbolos[100];
    for (int i = 0; i < 100; i++) {
        if (i < 26)
            simbolos[i] = 'A' + i;
        else if (i < 52)
            simbolos[i] = 'a' + (i - 26);
        else
            simbolos[i] = '*';
    }

    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1 &&
            memory->process[i].init >= 0 &&
            memory->process[i].init + memory->process[i].size <= memory->size) {

            for (int j = memory->process[i].init;
                 j < memory->process[i].init + memory->process[i].size && j < memory->size;
                 j++) {
                memoria[j] = simbolos[i];
            }
        }
    }
    
    printf("[");
    for (int i = 0; i < memory->size; i++) {
        char actual = memoria[i];
        if (actual == '-') {
            printf("%c", actual);
        } else {
            for (int j = 0; j < 100; j++) {
                if (memory->process[j].pid != -1 && simbolos[j] == actual) {
                    const char *color = colores[j % (sizeof(colores) / sizeof(char *))];
                    printf("%s%c%s", color, actual, RESET);
                    break;
                }
            }
        }
    }
    printf("]\n");

    printf("Procesos:\n");
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            const char *color = colores[i % (sizeof(colores) / sizeof(char *))];
            printf("  %s%c%s = PID %d\n",
                   color,
                   simbolos[i],
                   RESET,
                   memory->process[i].pid);
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
