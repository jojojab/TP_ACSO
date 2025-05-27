#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

// Parsea una línea respetando comillas dobles
void parse_arguments(const char *input, char **args, int *argc_out) {
    int argc = 0;
    const char *p = input;
    while (*p) {
        while (*p == ' ') p++; // Saltar espacios

        if (*p == '"') {
            p++; // Saltar comilla inicial
            const char *start = p;
            while (*p && *p != '"') p++; // Buscar comilla final
            int len = p - start;
            args[argc] = malloc(len + 1);
            strncpy(args[argc], start, len);
            args[argc][len] = '\0';
            argc++;
            if (*p == '"') p++;
        } else if (*p) {
            const char *start = p;
            while (*p && *p != ' ') p++;
            int len = p - start;
            args[argc] = malloc(len + 1);
            strncpy(args[argc], start, len);
            args[argc][len] = '\0';
            argc++;
        }
    }
    args[argc] = NULL;
    *argc_out = argc;
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1)
    {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break; // EOF
        }

        command[strcspn(command, "\n")] = '\0';

        char *token = strtok(command, "|");
        while (token != NULL)
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        if (command_count == 0) {
            break; // Línea vacía o EOF
        }

        /* You should start programming from here... */
        int pipefds[2 * (command_count - 1)];

        for (int i = 0; i < command_count - 1; i++) {
            if (pipe(pipefds + i * 2) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                // Redirigir entrada
                if (i != 0) {
                    if (dup2(pipefds[(i - 1) * 2], 0) < 0) {
                        perror("dup2 read");
                        exit(EXIT_FAILURE);
                    }
                }

                // Redirigir salida
                if (i != command_count - 1) {
                    if (dup2(pipefds[i * 2 + 1], 1) < 0) {
                        perror("dup2 write");
                        exit(EXIT_FAILURE);
                    }
                }

                // Cerrar todos los fds
                for (int j = 0; j < 2 * (command_count - 1); j++) {
                    close(pipefds[j]);
                }

                // Parsear argumentos respetando comillas
                char *args[100];
                int argc = 0;
                parse_arguments(commands[i], args, &argc);
                args[argc] = NULL;

                execvp(args[0], args);
                fprintf(stderr, "%s: command not found\n", args[0]);

                // Liberar memoria antes de salir
                for (int j = 0; j < argc; j++) {
                    free(args[j]);
                }

                exit(EXIT_FAILURE);
            }
        }

        // Padre cierra todos los pipe fds
        for (int i = 0; i < 2 * (command_count - 1); i++) {
            close(pipefds[i]);
        }

        // Esperar a todos los hijos
        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }

        command_count = 0;
    }

    return 0;
}
