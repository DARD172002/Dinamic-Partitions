#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "../include/share_memory.h"
#include <unistd.h>
#include "../include/global.h"

int main(){
    int size_of_memory;
    printf("Please insert of number lines of memory ");
    scanf("%d",&size_of_memory);
    //create and setting shared memory 
    int shared_fd= shm_open(SHARED_MEMORY, O_CREAT | O_RDWR, 0666); //open, frist intance create the file
    if (shared_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

if (ftruncate(shared_fd, sizeof(Shared_memory)) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
}
    
    Shared_memory *memory=mmap(NULL, sizeof(Shared_memory),PROT_READ| PROT_WRITE, MAP_SHARED,shared_fd,0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //initiallizce memory
    memory->size=size_of_memory;
    memory->free=size_of_memory;
    memory->number_of_proccess=0;

    //create semaphore
    sem_t *mem_sem = sem_open (SHARED_MEMORY, O_CREAT , 0666, 1);
    sem_t *mem_bit = sem_open(BITACORA, O_CREAT, 0666, 1);

    // Creation of bitacora file
    FILE *bitacora=fopen(BITACORA_FILE,"w");
    if (!bitacora) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(bitacora, "SIMULACIÓN INICIADA - Memoria: %d líneas\n", size_of_memory);
    fclose(bitacora);
    printf("Ambiente inicializado correctamente.\n");
    printf("Memoria: %d líneas\n", size_of_memory);
    printf("Semáforos y bitácora creados.\n");

    //free resources
    munmap(memory, sizeof(Shared_memory));
    close(shared_fd);
    return 0;

    
}