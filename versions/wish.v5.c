#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PATHS 10

char *paths[MAX_PATHS];
int path_count = 0;

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int batch_mode = 0;

    // Si hay más de un argumento, error
    if (argc > 2) {
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    }

    // Si se pasa un archivo, abrimos modo batch
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        batch_mode = 1;
    }

    // Variables para leer línea
    char *line = NULL;
    size_t len = 0;

    //Variables para path
    paths[0] = strdup("/bin");
    path_count = 1;
    while (1) {
        if (!batch_mode) {
            printf("wish> ");
            fflush(stdout);
        }

        ssize_t nread = getline(&line, &len, input);
        if (nread == -1) {
            break; // EOF
        }

        // Quitamos salto de línea final
        line[strcspn(line, "\n")] = '\0';

        // Ignorar líneas vacías
        if (strlen(line) == 0) continue;

        // Tokenizar la línea (máx. 10 args)
        char *args[10];
        int argc_cmd = 0;
        char *token = strtok(line, " \t");

        while (token != NULL && argc_cmd < 9) {
            args[argc_cmd++] = token;
            token = strtok(NULL, " \t");
        }
        args[argc_cmd] = NULL;
    	//Comando interno: exit
	    if (argc_cmd > 0 && strcmp(args[0], "exit") == 0) {
	        if (argc_cmd != 1) {
	            /* exit no acepta argumentos */
        	    fprintf(stderr, "An error has occurred\n");
        	    /* continuar al siguiente prompt */
        	    continue;
    	    } else {
        	    /* limpiar y salir */
        	    if (line) free(line);
        	        if (batch_mode && input) fclose(input);
        	            exit(0);
    	    }
	    }
	    // Comando interno: cd
	    if (argc_cmd > 0 && strcmp(args[0], "cd") == 0) {
    	    /* cd debe recibir exactamente un argumento (cd <dir>) */
		    if (argc_cmd != 2) {
           	    fprintf(stderr, "An error has occurred\n");
        	    continue;
    		}
	    	/* intentar cambiar de directorio */
	    	if (chdir(args[1]) != 0) {
		        fprintf(stderr, "An error has occurred\n");
	    	}
    		/* ya ejecutamos cd en el padre, no fork ni exec */
		    continue;
	    }
        // Comando interno: path
        if (strcmp(args[0], "path") == 0) {
            // Liberar rutas anteriores
            for (int i = 0; i < path_count; i++) {
                free(paths[i]);
            }
            path_count = 0;

            // Agregar nuevas rutas
            for (int i = 1; i < argc_cmd && i < MAX_PATHS; i++) {
                paths[path_count++] = strdup(args[i]);
            }

            continue;
        }

        // Crear proceso hijo
        pid_t pid = fork();

        if (pid == 0) {
            // Proceso hijo: intenta ejecutar el comando
            if (pid == 0) {
                // Proceso hijo: intenta buscar el comando en los paths
                for (int i = 0; i < path_count; i++) {
                    char full_path[256];
                    snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], args[0]);
                    execv(full_path, args);
                }
                // Si llega aquí, ningún path funcionó
                fprintf(stderr, "An error has occurred\n");
                exit(1);
            }

            // Si llega aquí, hubo error
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        } else if (pid > 0) {
            // Proceso padre: espera al hijo
            waitpid(pid, NULL, 0);
        } else {
            // Error al crear proceso
            fprintf(stderr, "An error has occurred\n");
        }
    }

    free(line);
    if (batch_mode) fclose(input);

    return 0;
}
