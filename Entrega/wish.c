#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_CMDS 10
#define MAX_PATHS 10

char *paths[MAX_PATHS];
int path_count = 0;
char error_message[30] = "An error has occurred\n";

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int execute_command(char *cmd_line) {
    // Tokenizar comando
    char *args[10];
    int argc_cmd = 0;
    char *token = strtok(cmd_line, " \t");
    while (token != NULL && argc_cmd < 9) {
        args[argc_cmd++] = token;
        token = strtok(NULL, " \t");
    }
    args[argc_cmd] = NULL;

    if (argc_cmd == 0) return 0;

    // Redirección
    int redirect = 0;
    char *outfile = NULL;
    for (int i = 0; i < argc_cmd; i++) {
        if (strcmp(args[i], ">") == 0) {
            redirect = 1;
            if (i + 1 >= argc_cmd || i + 2 != argc_cmd) {
                print_error();
                return -1;
            }
            outfile = args[i + 1];
            args[i] = NULL;
            argc_cmd = i;
            break;
        }
    }

    // Built-in: exit
    if (strcmp(args[0], "exit") == 0) {
        if (argc_cmd != 1) {
            print_error();
            return -1;
        } else {
            exit(0);
        }
    }

    // Built-in: cd
    if (strcmp(args[0], "cd") == 0) {
        if (argc_cmd != 2) {
            print_error();
            return -1;
        }
        if (chdir(args[1]) != 0) {
            print_error();
        }
        return 0;
    }

    // Built-in: path
    if (strcmp(args[0], "path") == 0) {
        for (int i = 0; i < path_count; i++) free(paths[i]);
        path_count = 0;
        for (int i = 1; i < argc_cmd && i < MAX_PATHS; i++) {
            paths[path_count++] = strdup(args[i]);
        }
        return 0;
    }

    // Crear hijo
    pid_t pid = fork();
    if (pid == 0) {
        // Hijo
        if (redirect == 1) {
            int fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
            if (fd < 0) {
                print_error();
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

        print_error();
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        print_error();
    }

    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN); //Para evitar procesos zombies
    FILE *input = stdin;
    int batch_mode = 0;

    // Si hay más de un argumento, error
    if (argc > 2) {
        print_error();
        exit(1);
    }

    // Si se pasa un archivo, abrimos modo batch
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            print_error();
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
        if (nread == -1) break;

        // Quitamos salto de línea final
        line[strcspn(line, "\n")] = '\0';

        // Ignorar líneas vacías
        if (strlen(line) == 0) continue;

    	// Separar por &
        char *cmds[MAX_CMDS];
        int cmd_count = 0;
        char *cmd = strtok(line, "&");
        while (cmd != NULL && cmd_count < MAX_CMDS) {
            cmds[cmd_count++] = cmd;
            cmd = strtok(NULL, "&");
        }

        pid_t pids[MAX_CMDS];
        int pid_count = 0;

        for (int i = 0; i < cmd_count; i++) {
            // Quitar espacios iniciales
            while (*cmds[i] == ' ' || *cmds[i] == '\t') cmds[i]++;
            if (strlen(cmds[i]) == 0) continue;

            // Verificar si es interno antes de fork
            char temp_cmd[256];
            strncpy(temp_cmd, cmds[i], sizeof(temp_cmd));
            temp_cmd[sizeof(temp_cmd) - 1] = '\0';

            char *first = strtok(temp_cmd, " \t");
            if (first && (strcmp(first, "cd") == 0 || strcmp(first, "exit") == 0 || strcmp(first, "path") == 0)) {
                execute_command(cmds[i]);
                continue;
            }

            // Ejecutar en proceso hijo
            pid_t pid = fork();
            if (pid == 0) {
                execute_command(cmds[i]);
                exit(0);
            } else if (pid > 0) {
                pids[pid_count++] = pid;
            } else {
                print_error();
            }
        }

        // Esperar todos los hijos
        for (int i = 0; i < pid_count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    for (int i = 0; i < path_count; i++) {
       free(paths[i]);
    }
    free(line);
    if (batch_mode) fclose(input);

    return 0;
}
