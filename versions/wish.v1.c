#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

        //Muestro el comando leido en pantalla
        printf("Comando leído: %s", line);
    }

    free(line);
    if (batch_mode) fclose(input);

    return 0;
}
