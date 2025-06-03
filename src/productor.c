#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include<stdbool.h>
#include "../include/share_memory.h"
#include "../include/global.h"
#include <time.h>

#define MAX_LINEAS_POR_HILO 10
int algoritmo = 0; // 1 = First in, 2 = Worst Fit, 3 = Best Fit

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
};  
char *obtener_hora_actual() {
    static char buffer[9];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return buffer;
}


void mostrar_memoria() {
    char memoria[memory->size];
    for (int i = 0; i < memory->size; i++) {
        memoria[i] = '-';
    }

    // Símbolos únicos
    char simbolos[100];
    for (int i = 0; i < 100; i++) {
        if (i < 26)
            simbolos[i] = 'A' + i;
        else if (i < 52)
            simbolos[i] = 'a' + (i - 26);
        else
            simbolos[i] = '*';
    }

    // Asignar símbolos a bloques
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

    // Imprimir memoria
    printf("\n== ESTADO DE LA MEMORIA (%d libres) ==\n[", memory->free);
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

    // Leyenda
    printf("Leyenda:\n");
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1 &&
            memory->process[i].init >= 0 &&
            memory->process[i].init + memory->process[i].size <= memory->size) {

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
//crea un mapa que facilita la busqueda de bloques libres
void marcar_ocupadas(int *ocupado, int size) {
    for (int i = 0; i < size; i++) {
        ocupado[i] = 0;
    }

    // Marcar posiciones ocupadas en memoria
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid != -1) {
            int inicio = memory->process[i].init;
            int fin = inicio + memory->process[i].size;
            if (inicio >= 0 && fin <= size) {
                for (int j = inicio; j < fin; j++) {
                    ocupado[j] = 1;
                }
            }
        }
    }
}

int best_fit(int requerido) {
    int ocupado[memory->size];
    marcar_ocupadas(ocupado, memory->size);

    int inicio = -1;
    int mejor_tam = memory->size + 1;  

    int i = 0;
    while (i < memory->size) {
        if (ocupado[i]) {
            i++;
            continue;
        }

        int inicio_bloque = i;
        int tam_bloque = 0;
        while (i < memory->size && !ocupado[i]) {
            tam_bloque++;
            i++;
        }

        if (tam_bloque >= requerido && tam_bloque < mejor_tam) {
            mejor_tam = tam_bloque;
            inicio = inicio_bloque;
        }
    }

    return inicio;
}


int worst_fit(int requerido) {
    int ocupado[memory->size];
    marcar_ocupadas(ocupado, memory->size);

    int inicio = -1;
    int mejor_tam = 0;

    int i = 0;
    while (i < memory->size) {
        // Saltar posiciones ocupadas
        if (ocupado[i]) {
            i++;
            continue;
        }

        // Encontrar el tamaño del bloque libre comenzando en i
        int inicio_bloque = i;
        int tam_bloque = 0;
        while (i < memory->size && !ocupado[i]) {
            tam_bloque++;
            i++;
        }

        // Se queda con el mejor bloque
        if (tam_bloque >= requerido && tam_bloque > mejor_tam) {
            mejor_tam = tam_bloque;
            inicio = inicio_bloque;
        }
    }

    return inicio; 
}

//first in
int fifo(int requerido) {
    int ocupado[memory->size];
    marcar_ocupadas(ocupado, memory->size);
    // Encuentra el primer bloque libre
    for (int i = 0; i <= memory->size - requerido; i++) {
        int libre = 1;
        for (int j = 0; j < requerido; j++) {
            if (ocupado[i + j]) {
                libre = 0; 
                break;
            }
        }
        if (libre) return i; //retorna la posicion del bloque 
    }

    return -1;  // No hay espacio
}


void *hilo_funcion(void *arg) {
    int id = *(int *)arg;
    int lines = (rand() % MAX_LINEAS_POR_HILO) + 1;
    int duracion = (rand() % 41) + 20;//20 a 60
    //int duracion = (rand() % 16) + 5;  // entre 5 y 20 segundos

    sem_wait(mem_sem);
    FILE *bitacora = fopen(BITACORA_FILE, "a");
    if (memory->free < lines) {
        printf("Hilo %d: Fallo - Fragmentación, no hay bloque contiguo de %d líneas.\n", id, lines);

        if (bitacora) {
            fprintf(bitacora, "[%s] PID %d - FALLO: Memoria insuficiente (necesita %d líneas, libres %d)\n",
                    obtener_hora_actual(), id, lines, memory->free);
            fclose(bitacora);
        }
        sem_post(mem_sem);
        free(arg);
        return NULL;
    }

    int inicio = -1;
    
    if (algoritmo == 1) {
        inicio = fifo(lines);
    } else if (algoritmo == 2) {
        inicio = worst_fit(lines);
    } else if (algoritmo == 3) {
        inicio = best_fit(lines);
    }


    if (inicio == -1 || inicio + lines > memory->size) {
        printf("Hilo %d: No se encontró espacio contiguo de %d líneas.\n", id, lines);
        if (bitacora) {
            fprintf(bitacora, "[%s] PID %d - FALLO: Fragmentación, sin bloque contiguo para %d líneas\n",
                    obtener_hora_actual(), id, lines);
            fclose(bitacora);
        }
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
            memory->process[i].time = duracion;
            memory->number_of_proccess++;
            memory->free -= lines;
            printf("Hilo %d: Reservó %d líneas desde %d.\n", id, lines, inicio);
            if (bitacora) {
                fprintf(bitacora, "[%s] PID %d - ASIGNACIÓN: %d líneas desde %d, va a esperar %ds\n",
                        obtener_hora_actual(), id, lines, inicio, duracion);
                fclose(bitacora);
            }
            break;
        }
    }

    //mostrar_memoria();
    sem_post(mem_sem);  

    sleep(duracion);

    // Liberar memoria
    sem_wait(mem_sem);
    int auxId = -1;
    int auxSize = 0;
    for (int i = 0; i < 100; i++) {
        if (memory->process[i].pid == id) {
            printf("Hilo %d: Liberando %d líneas desde %d.\n", id,
                   memory->process[i].size, memory->process[i].init);
            auxId = memory->process[i].init;
            auxSize = memory->process[i].size;
            memory->free += memory->process[i].size;
            memory->process[i].pid = -1;
            memory->process[i].init = -1;
            memory->process[i].size = 0;
            memory->process[i].time = 0;
            memory->number_of_proccess--;
            bitacora = fopen(BITACORA_FILE, "a");
            if (bitacora) {
                fprintf(bitacora, "[%s] PID %d - DESASIGNACIÓN de %d líneas desde %d\n",
                        obtener_hora_actual(), id,
                        auxSize, auxId);
                fclose(bitacora);
            }
            break;
        }
    }
    //mostrar_memoria();
    sem_post(mem_sem);

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
    bool elegir = true;
    while (elegir){
        printf("Seleccione el algoritmo de asignación de memoria:\n");
        printf("1. First in\n");
        printf("2. Worst Fit\n");
        printf("3. Best Fit\n");
        printf("Ingrese la opción deseada: ");
        scanf("%d", &algoritmo);

        
        if (algoritmo < 1 || algoritmo > 3) {
            printf("Opción inválida.\n");
            exit(EXIT_FAILURE);
        } else{
            elegir = false;
        }
    }
    FILE *bitacora = fopen(BITACORA_FILE, "a");
    if (bitacora) {
        fprintf(bitacora, "[%s] Algoritmo seleccionado: %s\n", obtener_hora_actual(),
                algoritmo == 1 ? "First in" : algoritmo == 2 ? "Worst Fit" : "Best Fit");
        fclose(bitacora);
    }
    int contador_id = 1;
    while (1) {
        int esperar = (rand() % 31) + 30;
        int *id = malloc(sizeof(int));
        *id = contador_id++;
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, hilo_funcion, id) != 0) {
            perror("pthread_create");
            free(id);
        } else {
            pthread_detach(hilo); 
        }

        sleep(esperar);  // tiempo de espera para crear hilos, en la version final cambia
        
    }

    shmdt(memory);
    return 0;
}
