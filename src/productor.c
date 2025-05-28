#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#include "../include/share_memory.h"
#include "../include/global.h"

#define MAX_LINEAS_POR_HILO 10

sem_t *mem_sem;
Shared_memory *memory;

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
};  // Hasta 13 colores, puedes duplicar si necesitas más

void mostrar_memoria() {
    char memoria[memory->size];
    for (int i = 0; i < memory->size; i++) {
        memoria[i] = '-';
    }

    // Asignar símbolo único a cada proceso
    char simbolos[100];
    for (int i = 0; i < 100; i++) {
        if (i < 26)
            simbolos[i] = 'A' + i;
        else if (i < 52)
            simbolos[i] = 'a' + (i - 26);
        else
            simbolos[i] = '*';
    }

    // Asignar a la memoria
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            for (int j = memory->process[i].init; j < memory->process[i].init + memory->process[i].size; j++) {
                memoria[j] = simbolos[i];
            }
        }
    }

    // Imprimir memoria con colores
    printf("\n== ESTADO DE LA MEMORIA (%d libres) ==\n[", memory->free);
    for (int i = 0; i < memory->size; i++) {
        char actual = memoria[i];
        if (actual == '-') {
            printf("%c", actual);  // Libre: sin color
        } else {
            // Buscar qué proceso lo usa y colorear
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

    // Leyenda
    printf("Leyenda:\n");
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            const char *color = colores[i % (sizeof(colores) / sizeof(char *))];
            printf("  %s%c%s = PID %d (%d líneas desde %d)\n",
                   color, simbolos[i], RESET,
                   memory->process[i].pid,
                   memory->process[i].size,
                   memory->process[i].init);
        }
    }
    printf("====================================\n\n");
}


void *hilo_funcion(void *arg) {
    int id = *(int *)arg;
    int lines = (rand() % MAX_LINEAS_POR_HILO) + 1;

    sem_wait(mem_sem);
    if (memory->free < lines) {
        printf("Hilo %d: No hay suficiente memoria (necesita %d líneas).\n", id, lines);
        sem_post(mem_sem);
        free(arg);
        return NULL;
    }

    // Buscar bloque contiguo
    int inicio = -1;
    for (int i = 0; i <= memory->size - lines; i++) {
        int libre = 1;
        for (int j = 0; j < 100; j++) {
            if (memory->process[j].pid != -1 &&
                i >= memory->process[j].init &&
                i < (memory->process[j].init + memory->process[j].size)) {
                libre = 0;
                break;
            }
        }
        if (libre) {
            inicio = i;
            break;
        }
    }

    if (inicio == -1) {
        printf("Hilo %d: No se encontró espacio contiguo de %d líneas.\n", id, lines);
        sem_post(mem_sem);
        free(arg);
        return NULL;
    }

    // Registrar proceso
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid == -1) {
            memory->process[i].pid = id;
            memory->process[i].init = inicio;
            memory->process[i].size = lines;
            memory->process[i].time = 0;
            memory->number_of_proccess++;
            memory->free -= lines;
            printf("Hilo %d: Reservó %d líneas desde %d.\n", id, lines, inicio);
            break;
        }
    }

    
    sem_post(mem_sem);

    // tiempo aleatorio del sleep
    int duracion = (rand() % 16) + 5;
    mostrar_memoria();
    sleep(duracion);

    // Liberar memoria
    sem_wait(mem_sem);
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid == id) {
            printf("Hilo %d: Liberando %d líneas desde %d.\n", id,
                   memory->process[i].size, memory->process[i].init);
            memory->free += memory->process[i].size;
            memory->process[i].pid = -1;
            memory->process[i].init = -1;
            memory->process[i].size = 0;
            memory->process[i].time = 0;
            memory->number_of_proccess--;
            break;
        }
    }
    
    sem_post(mem_sem);
    mostrar_memoria();
    free(arg);
    return NULL;
}

int main() {
    srand(time(NULL));

    // Conexión a la memoria compartida
    int shmid = shmget(SHM_KEY, sizeof(Shared_memory), 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    memory = (Shared_memory *)shmat(shmid, NULL, 0);
    if (memory == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    mem_sem = sem_open(SHARED_MEMORY, 0);
    if (mem_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int contador_id = 1;
    while (1) {
        int *id = malloc(sizeof(int));
        *id = contador_id++;
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, hilo_funcion, id) != 0) {
            perror("pthread_create");
            free(id);
        } else {
            pthread_detach(hilo); 
        }

        sleep(5);  // tiempo de espera para crear hilos, en la version final cambia
    }

    shmdt(memory);
    return 0;
}
