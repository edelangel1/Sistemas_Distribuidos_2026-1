#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h> 
#include<sys/wait.h>  

void crear_hijos(int n) {
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Código del hijo
            printf("Proceso Hijo: PID = %d, Padre PID = %d\n", getpid(), getppid());
            exit(0);  // El hijo termina aquí
        }
    }
}

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo 1: Crea 3 hijos
        printf("Proceso Hijo 1: PID = %d, Padre PID = %d\n", getpid(), getppid());

        // Crear 3 procesos hijos
        for (int i = 0; i < 3; i++) {
            pid_t pid_2 = fork();

            if (pid_2 == 0) {
                // Proceso hijo de los 3 procesos del hijo 1: Crea 2 procesos hijos
                printf("Proceso Hijo 2-%d: PID = %d, Padre PID = %d\n", i + 1, getpid(), getppid());

                // Crear 2 procesos hijos para cada uno
                crear_hijos(2);

                exit(0);  // El hijo 2 termina aquí
            }
        }
        
        // El proceso padre de nivel 1 (hijo 1) espera a sus hijos
        for (int i = 0; i < 3; i++) {
            wait(NULL);
        }
    } else {
        // Proceso padre: Espera a su hijo
        printf("Proceso Padre: PID = %d\n", getpid());
        wait(NULL);
    }

    return 0;
}
