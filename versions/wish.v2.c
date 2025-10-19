#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

        // Crear proceso hijo
        pid_t pid = fork();

        if (pid == 0) {
            // Proceso hijo: intenta ejecutar el comando
            execvp(args[0], args);
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
