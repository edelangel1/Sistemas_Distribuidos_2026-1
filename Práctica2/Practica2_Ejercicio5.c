#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>      // Para manejar directorios
#include <sys/stat.h>    // Para stat() y mkdir()
#include <sys/types.h>   // Para tipos de datos usados por stat()

// Estructura para los argumentos del hilo 1 (contador)
typedef struct {
    const char* nombre_archivo;
    const char* cadena_a_buscar;
} ArgsContador;

// Estructura para los argumentos del hilo 2 (copiador)
typedef struct {
    const char* dir_destino;
} ArgsCopiador;

// Estructura para los argumentos del hilo 3 (reporte)
typedef struct {
    long ocurrencias;
    long archivos_copiados;
    const char* archivo_reporte;
} ArgsReporte;


// HILO 1: Contabiliza las ocurrencias de una cadena en un archivo
void* contar_ocurrencias(void* arg) {
    ArgsContador* args = (ArgsContador*)arg;
    FILE* archivo = fopen(args->nombre_archivo, "r");
    if (!archivo) {
        perror("Hilo 1: No se pudo abrir el archivo de búsqueda");
        pthread_exit((void*) -1);
    }

    char linea[1024];
    long contador = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        char* pos = linea;
        // strstr encuentra la primera ocurrencia de la subcadena
        while ((pos = strstr(pos, args->cadena_a_buscar)) != NULL) {
            contador++;
            pos++; // Avanzar para no encontrar la misma ocurrencia infinitamente
        }
    }

    fclose(archivo);
    printf("Hilo 1: Búsqueda completada. Se encontraron %ld ocurrencias.\n", contador);
    pthread_exit((void*) contador);
}


// HILO 2: Copia los archivos del directorio actual a un subdirectorio
void* copiar_archivos_directorio(void* arg) {
    ArgsCopiador* args = (ArgsCopiador*)arg;
    DIR* dir;
    struct dirent* entrada;
    long archivos_copiados = 0;

    // Crear el directorio de destino si no existe
    struct stat st = {0};
    if (stat(args->dir_destino, &st) == -1) {
        // El directorio no existe, intentar crearlo
        if (mkdir(args->dir_destino, 0755) == -1) {
            perror("Hilo 2: No se pudo crear el directorio de destino");
            pthread_exit((void*) -1);
        }
        printf("Hilo 2: Directorio '%s' creado.\n", args->dir_destino);
    }

    // Abrir el directorio actual "."
    if ((dir = opendir(".")) == NULL) {
        perror("Hilo 2: No se pudo abrir el directorio actual");
        pthread_exit((void*) -1);
    }

    printf("Hilo 2: Iniciando copia de archivos a '%s'...\n", args->dir_destino);

    while ((entrada = readdir(dir)) != NULL) {
        char ruta_origen[512    ];
        snprintf(ruta_origen, sizeof(ruta_origen), "%s", entrada->d_name);

        struct stat info_archivo;
        // Usamos stat() para obtener información del archivo/directorio.
        if (stat(ruta_origen, &info_archivo) == -1) {
            // Si no podemos obtener info, lo saltamos.
            continue;
        }

        // Usamos la macro S_ISREG() para verificar si es un archivo regular.
        if (S_ISREG(info_archivo.st_mode)) {
            char ruta_destino[512];
            snprintf(ruta_destino, sizeof(ruta_destino), "%s/%s", args->dir_destino, entrada->d_name);
            
            // Evitar que el programa se copie a sí mismo (si el ejecutable está en el dir)
            if (strcmp(ruta_origen, "reporte.txt") == 0 || strcmp(ruta_origen, "a.out") == 0 || strcmp(ruta_origen, "main") == 0 || strcmp(ruta_origen, "multitarea") == 0) {
                continue;
            }

            FILE* f_origen = fopen(ruta_origen, "rb");
            FILE* f_destino = fopen(ruta_destino, "wb");

            if (f_origen && f_destino) {
                char buffer[4096];
                size_t bytes_leidos;
                while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), f_origen)) > 0) {
                    fwrite(buffer, 1, bytes_leidos, f_destino);
                }
                fclose(f_origen);
                fclose(f_destino);
                archivos_copiados++;
                printf("  -> Copiado: %s\n", ruta_origen);
            } else {
                if(f_origen) fclose(f_origen);
                if(f_destino) fclose(f_destino);
            }
        }
    }
    closedir(dir);
    
    printf("Hilo 2: Copia completada. Se copiaron %ld archivos.\n", archivos_copiados);
    pthread_exit((void*) archivos_copiados);
}


// HILO 3: Genera un archivo con los resultados de los otros dos hilos
void* generar_reporte(void* arg) {
    ArgsReporte* args = (ArgsReporte*)arg;
    FILE* archivo = fopen(args->archivo_reporte, "w");
    if (!archivo) {
        perror("Hilo 3: No se pudo crear el archivo de reporte");
        pthread_exit((void*)-1);
    }
    
    printf("Hilo 3: Generando reporte en '%s'...\n", args->archivo_reporte);
    
    fprintf(archivo, "--- Reporte de Ejecución ---\n\n");
    fprintf(archivo, "Resultado del Hilo 1 (Contador de Cadenas):\n");
    fprintf(archivo, " - Se encontraron %ld ocurrencias.\n\n", args->ocurrencias);
    
    fprintf(archivo, "Resultado del Hilo 2 (Copiador de Archivos):\n");
    fprintf(archivo, " - Se copiaron %ld archivos.\n", args->archivos_copiados);
    
    fclose(archivo);
    printf("Hilo 3: Reporte generado exitosamente.\n");
    pthread_exit(NULL);
}


// PROGRAMA PRINCIPAL
int main() {
    pthread_t hilo1, hilo2, hilo3;
    void* resultado_hilo1;
    void* resultado_hilo2;

    // --- Definición de argumentos ---
    ArgsContador args1 = {"archivo_busqueda.txt", "de"};
    ArgsCopiador args2 = {"copias"};

    printf("Programa Principal: Creando Hilo 1 y Hilo 2.\n");

    // --- Creación de los hilos 1 y 2 ---
    pthread_create(&hilo1, NULL, contar_ocurrencias, &args1);
    pthread_create(&hilo2, NULL, copiar_archivos_directorio, &args2);

    // --- Sincronización: esperar a que los hilos 1 y 2 terminen ---
    pthread_join(hilo1, &resultado_hilo1);
    pthread_join(hilo2, &resultado_hilo2);

    printf("Programa Principal: Hilo 1 y Hilo 2 han terminado.\n");

    // --- Preparar y crear el hilo 3 con los resultados ---
    ArgsReporte args3 = {
        .ocurrencias = (long)resultado_hilo1,
        .archivos_copiados = (long)resultado_hilo2,
        .archivo_reporte = "reporte.txt"
    };
    
    pthread_create(&hilo3, NULL, generar_reporte, &args3);

    // --- Esperar al hilo 3 ---
    pthread_join(hilo3, NULL);

    printf("Programa Principal: Todos los hilos han completado sus tareas. Fin.\n");

    return 0;
}