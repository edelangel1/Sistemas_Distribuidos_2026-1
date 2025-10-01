#include <stdio.h>
#include <pthread.h>

void *funcionHilo(void *arg);

int main(void)
{
    pthread_t id_hilo;
    int arg[2] = {2, 3};

    printf("Creación del hilo...\n");
    if (pthread_create(&id_hilo, NULL, funcionHilo, (void *)arg) != 0) {
        perror("pthread_create");
        return 1;
    }

    printf("Hilo creado, esperando su finalización...\n");
    pthread_join(id_hilo, NULL);

    printf("Hilo finalizado...\n valor 1: %d\n valor 2: %d\n", arg[0], arg[1]);
    return 0;
}

void *funcionHilo(void *argu)
{
    int *args = (int *)argu;
    printf("Valor 1: %d\n Valor 2: %d\n", args[0], args[1]);
    args[0] = 7;
    args[1] = 8;
    pthread_exit(NULL);
}
