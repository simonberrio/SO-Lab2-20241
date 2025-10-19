#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_PATHS 10
#define MAX_ARGS 10
#define MAX_COMMANDS 10

char *paths[MAX_PATHS];
int path_count = 0;

void ejecutar_comando(char *line) {
    // Tokenizar la línea
    char *args[MAX_ARGS];
    int argc_cmd = 0;
    char *token = strtok(line, " \t");
    while (token != NULL && argc_cmd < MAX_ARGS - 1) {
        args[argc_cmd++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc_cmd] = NULL;

    if (argc_cmd == 0) return; // línea vacía

    // Comandos internos
    // exit
    if (strcmp(args[0], "exit") == 0) {
        if (argc_cmd != 1) {
            fprintf(stderr, "An error has occurred\n");
            return;
        }
        exit(0);
    }

    // cd
    if (strcmp(args[0], "cd") == 0) {
        if (argc_cmd != 2) {
            fprintf(stderr, "An error has occurred\n");
            return;
        }
        if (chdir(args[1]) != 0) {
            fprintf(stderr, "An error has occurred\n");
        }
        return;
    }

    // path
    if (strcmp(args[0], "path") == 0) {
        for (int i = 0; i < path_count; i++) free(paths[i]);
        path_count = 0;
        for (int i = 1; i < argc_cmd && i < MAX_PATHS; i++) {
            paths[path_count++] = strdup(args[i]);
        }
        return;
    }

    // Redirección de salida
    int redirect = 0;
    char *outfile = NULL;

    for (int i = 0; i < argc_cmd; i++) {
        if (strcmp(args[i], ">") == 0) {
            redirect = 1;
            if (i + 1 >= argc_cmd || i + 2 != argc_cmd) {
                fprintf(stderr, "An error has occurred\n");
                return;
            }
            outfile = args[i + 1];
            args[i] = NULL;
            break;
        }
    }

    // Proceso hijo
    pid_t pid = fork();
    if (pid == 0) {
        if (redirect == 1) {
            int fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
            if (fd < 0) {
                fprintf(stderr, "An error has occurred\n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        for (int i = 0; i < path_count; i++) {
            char full_path[256];
            snprintf(full_path, sizeof(full_path), "%s/%s", paths[i], args[0]);
            execv(full_path, args);
        }

        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else if (pid < 0) {
        fprintf(stderr, "An error has occurred\n");
    } else {
        // Padre: no espera aquí (esperará después en paralelo)
    }
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN); //Para evitar procesos zombies
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

        // Dividir la línea por '&' (comandos en paralelo)
        char *commands[MAX_COMMANDS];
        int num_cmds = 0;
        char *cmd = strtok(line, "&");
        while (cmd != NULL && num_cmds < MAX_COMMANDS) {
            commands[num_cmds++] = cmd;
            cmd = strtok(NULL, "&");
        }

        pid_t pids[MAX_COMMANDS];

        for (int i = 0; i < num_cmds; i++) {
            // Quitar espacios iniciales y finales
            while (*commands[i] == ' ') commands[i]++;
            char *end = commands[i] + strlen(commands[i]) - 1;
            while (end > commands[i] && *end == ' ') *end-- = '\0';

            if (strlen(commands[i]) == 0) continue;

            pid_t pid = fork();
            if (pid == 0) {
                ejecutar_comando(commands[i]);
                exit(0);
            } else if (pid > 0) {
                pids[i] = pid;
            } else {
                fprintf(stderr, "An error has occurred\n");
            }
        }

        // Esperar a todos los hijos
        for (int i = 0; i < num_cmds; i++) {
            if (pids[i] > 0) {
                waitpid(pids[i], NULL, 0);
            }
        }
    }

    free(line);
    if (batch_mode) fclose(input);

    return 0;
}
