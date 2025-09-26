#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

// El struct del Token no cambia
typedef struct {
    pid_t target_pid;
    int cycles_left;
    char message[256];
} Token;

void node_logic(int node_id, int read_fd, int write_fd, int num_procs, pid_t pids[]) {
    // El mensaje de identificación se imprime una sola vez.
    printf("-> Nodo %d (PID: %d, Padre: %d) listo y esperando tokens.\n", node_id, getpid(), getppid());
    fflush(stdout);

    while (1) {
        Token token;
        ssize_t bytes_read = read(read_fd, &token, sizeof(Token));
        if (bytes_read <= 0) break;

        if (node_id == 0) {
            token.cycles_left--;
            printf("\n--- VUELTA COMPLETADA. Quedan %d. --- (Token recibido por Nodo 0)\n", token.cycles_left);
            fflush(stdout);

            if (token.cycles_left < 0) {
                // Notificar al anillo que debe terminar enviando un último token
                // y luego salir del bucle.
                write(write_fd, &token, sizeof(Token));
                break;
            }

            int random_target_node = rand() % num_procs;
            token.target_pid = pids[random_target_node];
            snprintf(token.message, sizeof(token.message), "Mensaje para el Nodo %d", random_target_node);
        }

        printf("   Nodo %d: Token recibido. Destino: PID %d.\n", node_id, token.target_pid);
        fflush(stdout);

        if (token.target_pid == getpid()) {
            printf("Nodo %d: ¡Mensaje para mí! -> \"%s\"\n", node_id, token.message);
            fflush(stdout);
        }
        
        // Siempre se pasa el token al siguiente.
        write(write_fd, &token, sizeof(Token));
        
        // Si el token que recibimos indica fin, terminamos después de pasarlo.
        if(token.cycles_left < 0) break;
    }

    close(read_fd);
    close(write_fd);
    printf("<- Nodo %d (PID %d) terminando.\n", node_id, getpid());
    fflush(stdout);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <numero_de_procesos>\n", argv[0]);
        return 1;
    }
    int num_procs = atoi(argv[1]);
    if (num_procs <= 1) {
        fprintf(stderr, "El número de procesos debe ser mayor que 1.\n");
    }

    printf("Iniciando simulación con %d procesos.\n", num_procs);

    int pipes[num_procs][2];
    for (int i = 0; i < num_procs; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe"); return 1;
        }
    }

    pid_t pids[num_procs];
    pids[0] = getpid();
    
    for (int i = 1; i < num_procs; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork"); return 1;
        }

        if (pid == 0) { // --- Código del Proceso Hijo ---
            // Un hijo es el Nodo 'i'. Lee de la tubería 'i-1' y escribe en la 'i'.
            int read_fd = pipes[i - 1][0];
            int write_fd = pipes[i][1];
            
            // Cerrar todas las demás tuberías que no usa.
            close(pipes[i-1][1]);
            close(pipes[i][0]);
            for(int j=0; j<num_procs; ++j) {
                if (j != i-1 && j != i) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            
            // FASE 1: Recibir y propagar la lista de PIDs.
            pid_t all_pids[num_procs];
            read(read_fd, all_pids, sizeof(pid_t) * num_procs);
            write(write_fd, all_pids, sizeof(pid_t) * num_procs);
            
            // FASE 2: Iniciar la lógica principal del token.
            node_logic(i, read_fd, write_fd, num_procs, all_pids);
            exit(0);
        } else {
            pids[i] = pid;
        }
    }

    // --- Código del Proceso Padre (Nodo 0) ---
    srand(time(NULL));
    // El padre lee de la última tubería y escribe en la primera.
    int read_fd = pipes[num_procs - 1][0];
    int write_fd = pipes[0][1];
    
    // Cerrar las demás tuberías.
    close(pipes[num_procs-1][1]);
    close(pipes[0][0]);
    for(int i=1; i<num_procs-1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // FASE 1: Iniciar la propagación de PIDs y esperar a que dé la vuelta.
    printf("\nPADRE: Iniciando fase de configuración (propagando PIDs)...\n");
    write(write_fd, pids, sizeof(pid_t) * num_procs); // Escribe la lista
    read(read_fd, pids, sizeof(pid_t) * num_procs);  // Espera a que regrese
    printf("PADRE: Configuración completada. Todos los nodos tienen el mapa de PIDs.\n\n");

    // FASE 2: Crear e inyectar el primer token para iniciar la operación.
    Token initial_token;
    initial_token.cycles_left = 3;
    int first_target_node = rand() % num_procs;
    initial_token.target_pid = pids[first_target_node];
    snprintf(initial_token.message, sizeof(initial_token.message), "Mensaje para el Nodo %d", first_target_node);
    write(write_fd, &initial_token, sizeof(Token));

    // El padre también entra en la lógica principal.
    node_logic(0, read_fd, write_fd, num_procs, pids);

    // Esperar a que todos los hijos terminen.
    for (int i = 1; i < num_procs; ++i) {
        wait(NULL);
    }

    printf("\nSimulación finalizada.\n");
    return 0;
}