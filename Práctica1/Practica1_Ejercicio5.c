#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    // Mensaje a escribir y nombre del archivo de salida
    const char *palabras[] = {"Hola", "esta", "es", "mi", "práctica", "uno"};
    const char *nombre_archivo = "mensaje.txt";
    const int num_hijos = 5;
    pid_t pid_hijo;

    // 1. Preparar el archivo (limpiarlo si existe)
    FILE *archivo = fopen(nombre_archivo, "w");
    if (archivo == NULL) {
        perror("Error al preparar el archivo");
        return 1;
    }
    fclose(archivo);

    // 2. Crear los 5 procesos hijos
    for (int i = 0; i < num_hijos; i++) {
        pid_hijo = fork();

        if (pid_hijo < 0) {
            perror("La llamada a fork() falló");
            return 1;
        }

        if (pid_hijo == 0) {
            // Proceso hijo escribe su palabra
            FILE *archivo_hijo = fopen(nombre_archivo, "a");
            if (archivo_hijo == NULL) exit(1);

            fprintf(archivo_hijo, "%s ", palabras[i]);
            fclose(archivo_hijo);
            exit(0);
        }
    }

    // 3. El padre espera a que todos los hijos terminen
    for (int i = 0; i < num_hijos; i++) {
        wait(NULL);
    }

    // 4. El padre escribe la última palabra
    archivo = fopen(nombre_archivo, "a");
    if (archivo == NULL) return 1;
    fprintf(archivo, "%s", palabras[5]); // "uno"
    fclose(archivo);

    // 5. Mensaje final
    printf("Archivo creado correctamente\n");

    return 0;
}
