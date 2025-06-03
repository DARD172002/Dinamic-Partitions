#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/share_memory.h"
#include "../include/global.h"
#define PRODUCTOR_EXE "productor"
#define ESPIA_EXE "spy"
#define INIT "init"
/*
void kill_processes() {
    printf("Terminando procesos activos...\n");
    
    // Comando de terminación mejorado
    int ret1 = system("pkill -f " PRODUCTOR_EXE " 2>/dev/null");
    if (WEXITSTATUS(ret1) == 0) {
        printf("Procesos productores terminados.\n");
    } else if (WEXITSTATUS(ret1) == 1) {
        printf("No se encontraron productores activos.\n");
    } else {
        printf("Error desconocido al terminar productores (Código %d)\n", WEXITSTATUS(ret1));
    }

    int ret2 = system("pkill -f " ESPIA_EXE " 2>/dev/null");
    if (WEXITSTATUS(ret2) == 0) {
        printf("Procesos espía terminados.\n");
    } else if (WEXITSTATUS(ret2) == 1) {
        printf("No se encontraron espías activos.\n");
    } else {
        printf("Error desconocido al terminar espías (Código %d)\n", WEXITSTATUS(ret2));
    }
}
*/

void kill_processes() {
    printf("Terminando procesos activos...\n");
    
    int ret1 = system("pkill -f " PRODUCTOR_EXE " 2>/dev/null");
    switch(WEXITSTATUS(ret1)) {
        case 0: printf("Productores terminados correctamente\n"); break;
        case 1: printf("No había productores activos\n"); break;
        default: printf("Error inesperado al terminar productores (%d)\n", WEXITSTATUS(ret1));
    }

    int ret2 = system("pkill -f " ESPIA_EXE " 2>/dev/null");
    switch(WEXITSTATUS(ret2)) {
        case 0: printf("Espías terminados correctamente\n"); break;
        case 1: printf("No había espías activos\n"); break;
        default: printf("Error inesperado al terminar espías (%d)\n", WEXITSTATUS(ret2));
    }
}
int main() {
    //get ID of segment of shared memory

    kill_processes();
    
    int shmid = shmget(SHM_KEY, sizeof(Shared_memory), 0666);
    if (shmid == -1) {
        perror("shmget");
        fprintf(stderr, "¿Ya se liberó la memoria compartida?\n");
    } else {
        //detele shared memory segment
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(EXIT_FAILURE);
        } else {
            printf("Memoria compartida eliminada correctamente.\n");
        }
    }

    // Close and delete semaphore 
    if (sem_unlink(SHARED_MEMORY) == -1) {
        perror("sem_unlink SHARED_MEMORY");
    } else {
        printf("Semáforo SHARED_MEMORY eliminado.\n");
    }

    /*if (sem_unlink(BITACORA) == -1) {
        perror("sem_unlink BITACORA");
    } else {
        printf("Semáforo BITACORA eliminado.\n");
    }*/
    //write in the bitacore the simulation has finished
    sem_t *mem_bit = sem_open(BITACORA, O_CREAT, 0666, 1); // por si ya se eliminó
    if (mem_bit != SEM_FAILED) {
        sem_wait(mem_bit);
        FILE *bitacora = fopen(BITACORA_FILE, "a");
        if (bitacora) {
            fprintf(bitacora, "SIMULACIÓN FINALIZADA.\n");
            fclose(bitacora);
        } else {
            perror("fopen bitacora");
        }
        sem_post(mem_bit);
        sem_close(mem_bit);
        sem_unlink(BITACORA); // asegúrate de borrarlo otra vez por si lo reabrió
    }

    printf("Finalización completada.\n");
    return 0;
}
