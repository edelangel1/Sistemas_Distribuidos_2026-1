#include <stdio.h>
#include <pthread.h>

void *funcionHilo(void *args);

int main(void)
{
    pthread_t id_hilo;
    printf("Creación del hilo...\n");
    pthread_create(&id_hilo, NULL, funcionHilo, NULL);
    printf("Hilo creado, esperando su finalización...\n");
    pthread_join(id_hilo, NULL);
    printf("Hilo finalizado...\n");
    return 0;
}

void *funcionHilo(void *argu)
{
    printf("Dentro del hilo...\n");
    pthread_exit(NULL);
}
