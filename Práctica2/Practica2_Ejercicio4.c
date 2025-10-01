#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      // Para fork() y getpid()
#include <pthread.h>     // Para la biblioteca de hilos Pthreads
#include <sys/wait.h>    // Para wait()

// Prototipos de las funciones que ejecutarán los hilos
void* rutina_hilo_nivel1(void* arg);
void* rutina_hilo_terminal(void* arg);

// NIVEL 3: HILOS TERMINALES
// Tarea: Cada uno de los 6 hilos terminales ejecuta esta función.
void* rutina_hilo_terminal(void* arg) {
    printf("\t\t\t--> Hilo Terminal [%lu]: Hola mundo\n", (unsigned long)pthread_self());
    return NULL;
}

// NIVEL 2: HILOS PADRE INTERMEDIOS
// Tarea: Cada uno de los 3 hilos de nivel 1 ejecuta esta función.
void* rutina_hilo_nivel1(void* arg) {
    pthread_t hilos_terminales[2];
    printf("\t\t>> Hilo Nivel 1 [%lu] creando 2 hilos terminales...\n", (unsigned long)pthread_self());

    // Crear dos hilos terminales
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&hilos_terminales[i], NULL, rutina_hilo_terminal, NULL) != 0) {
            perror("Fallo al crear hilo terminal");
            exit(EXIT_FAILURE);
        }
    }

    // Imprimir los identificadores de los hilos creados
    printf("\t\t>> Hilo Nivel 1 [%lu] creó los hilos con ID: [%lu, %lu]\n",
           (unsigned long)pthread_self(),
           (unsigned long)hilos_terminales[0],
           (unsigned long)hilos_terminales[1]);

    // Esperar a que los hilos terminales finalicen
    for (int i = 0; i < 2; i++) {
        pthread_join(hilos_terminales[i], NULL);
    }
    
    return NULL;
}


// NIVEL 1: PROCESO HIJO
// Tarea: El proceso hijo ejecuta esta función.
void ejecutar_logica_hijo() {
    pthread_t hilos_nivel1[3];
    printf("\t> Proceso Hijo (PID: %d) creando 3 hilos...\n", getpid());

    // Crear los tres hilos de nivel 1
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&hilos_nivel1[i], NULL, rutina_hilo_nivel1, NULL) != 0) {
            perror("Fallo al crear hilo de nivel 1");
            exit(EXIT_FAILURE);
        }
    }

    // Imprimir los identificadores de los hilos creados
    printf("\t> Proceso Hijo (PID: %d) creó los hilos con ID: [%lu, %lu, %lu]\n",
           getpid(),
           (unsigned long)hilos_nivel1[0],
           (unsigned long)hilos_nivel1[1],
           (unsigned long)hilos_nivel1[2]);

    // Esperar a que los tres hilos de nivel 1 finalicen
    for (int i = 0; i < 3; i++) {
        pthread_join(hilos_nivel1[i], NULL);
    }

    printf("\t> Proceso Hijo (PID: %d) ha terminado.\n", getpid());
}


// NIVEL 0: PROCESO PADRE
// Tarea: El programa principal inicia aquí.
int main() {
    pid_t pid_hijo;

    printf("Proceso Padre (PID: %d) va a crear un proceso hijo.\n", getpid());

    pid_hijo = fork(); // Creación del proceso hijo

    if (pid_hijo < 0) {
        // Error al crear el proceso
        perror("Fallo en fork");
        return 1;
    }

    if (pid_hijo == 0) {
        // --- Este código SÓLO lo ejecuta el proceso hijo ---
        ejecutar_logica_hijo();
        exit(0); // El hijo termina aquí.
    } else {
        // --- Este código SÓLO lo ejecuta el proceso padre ---
        printf("Proceso Padre (PID: %d) creó al Proceso Hijo con PID: %d\n", getpid(), pid_hijo);

        // El padre espera a que el hijo termine toda su jerarquía de hilos
        wait(NULL);

        printf("Proceso Padre (PID: %d) ha terminado.\n", getpid());
    }

    return 0;
}