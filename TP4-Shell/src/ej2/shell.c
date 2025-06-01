#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 100

// Quita espacios iniciales y finales
char *trim_whitespace(char *str) {
    while (*str == ' ') str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    *(end + 1) = '\0';
    return str;
}

// Parsea una línea respetando comillas dobles
void parse_arguments(const char *input, char **args, int *argc_out) {
    int argc = 0;
    const char *p = input;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (*p == '"') {
            p++;
            const char *start = p;
            while (*p && *p != '"') p++;
            int len = p - start;
            args[argc] = malloc(len + 1);
            strncpy(args[argc], start, len);
            args[argc][len] = '\0';
            argc++;
            if (*p == '"') p++;
        } else {
            const char *start = p;
            while (*p && *p != ' ' && *p != '\t') p++;
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

// Parsea la línea en comandos separados por '|', sin strtok
// Retorna la cantidad de comandos encontrados
int split_commands(char *line, char **commands) {
    int count = 0;
    char *start = line;
    char *p = line;

    while (*p) {
        if (*p == '|') {
            *p = '\0'; // separa la cadena
            commands[count++] = trim_whitespace(start);
            start = p + 1;
        }
        p++;
    }
    // último comando
    if (*start != '\0') {
        commands[count++] = trim_whitespace(start);
    }
    return count;
}

void ejecutar_linea(char *command_line) {
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    // Revisar || como error de sintaxis
    for (int i = 0; command_line[i]; i++) {
        if (command_line[i] == '|' && command_line[i+1] == '|') {
            fprintf(stderr, "-bash: syntax error near unexpected token `||'\n");
            return;
        }
        if (command_line[i] == '|' && command_line[i+2] == '|') {
            fprintf(stderr, "-bash: syntax error near unexpected token `|'\n");
            return;
        }
    }

    command_count = split_commands(command_line, commands);
    if (command_count == 0) return;

    // Comando interno exit
    if (command_count == 1 && strcmp(commands[0], "exit") == 0) {
        exit(0);
    }

    int pipefds[2*(command_count-1)];

    for (int i = 0; i < command_count-1; i++) {
        if (pipe(pipefds + i*2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < command_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // hijo

            // stdin para todos excepto primer comando
            if (i != 0) {
                if (dup2(pipefds[(i-1)*2], STDIN_FILENO) < 0) {
                    perror("dup2 stdin");
                    exit(EXIT_FAILURE);
                }
            }
            // stdout para todos excepto último comando
            if (i != command_count - 1) {
                if (dup2(pipefds[i*2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 stdout");
                    exit(EXIT_FAILURE);
                }
            }

            // cerrar todos los fds de pipes en hijo
            for (int j = 0; j < 2*(command_count-1); j++) {
                close(pipefds[j]);
            }

            char *args[MAX_ARGS];
            int argc = 0;
            parse_arguments(commands[i], args, &argc);
            args[argc] = NULL;

            // exit en pipeline
            if (strcmp(args[0], "exit") == 0) {
                for (int j = 0; j < argc; j++) free(args[j]);
                exit(0);
            }

            execvp(args[0], args);

            fprintf(stderr, "bash: %s: command not found\n", args[0]);
            for (int j = 0; j < argc; j++) free(args[j]);
            exit(EXIT_FAILURE);
        }
    }

    // padre cierra todos pipes
    for (int i = 0; i < 2*(command_count-1); i++) {
        close(pipefds[i]);
    }

    // padre espera hijos
    for (int i = 0; i < command_count; i++) {
        wait(NULL);
    }
}

int main() {
    char command[4096];

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = '\0';

        if (strlen(command) == 0) continue;

        ejecutar_linea(command);
    }
    return 0;
}
