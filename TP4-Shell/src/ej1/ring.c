#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_COMMANDS 200

// Quita espacios iniciales y finales
char *trim_whitespace(char *str) {
    while (*str == ' ') str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    *(end + 1) = '\0';
    return str;
}

// Parsea una línea respetando comillas dobles y simples
void parse_arguments(const char *input, char **args, int *argc_out) {
    int argc = 0;
    const char *p = input;

    while (*p) {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            const char *start = p;
            while (*p && *p != quote) p++;
            int len = p - start;
            args[argc] = malloc(len + 1);
            strncpy(args[argc], start, len);
            args[argc][len] = '\0';
            argc++;
            if (*p == quote) p++;
        } else {
            const char *start = p;
            while (*p && *p != ' ') p++;
            int len = p - start;
            if (len > 0) {
                args[argc] = malloc(len + 1);
                strncpy(args[argc], start, len);
                args[argc][len] = '\0';
                argc++;
            }
        }
    }

    args[argc] = NULL;
    *argc_out = argc;
}

int main() {
    char command[1024];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (fgets(command, sizeof(command), stdin) == NULL) break;

        command[strcspn(command, "\n")] = '\0';

        command_count = 0;
        char *saveptr;
        char *token = strtok_r(command, "|", &saveptr);
        while (token != NULL) {
            token = trim_whitespace(token);
            if (*token == '\0') {
                fprintf(stderr, "Error: comando vacío entre pipes\n");
                command_count = 0;
                break;
            }
            commands[command_count++] = token;
            token = strtok_r(NULL, "|", &saveptr);
        }

        if (command_count == 0) continue;

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
                if (i != 0) {
                    if (dup2(pipefds[(i - 1) * 2], 0) < 0) {
                        perror("dup2 read");
                        exit(EXIT_FAILURE);
                    }
                }
                if (i != command_count - 1) {
                    if (dup2(pipefds[i * 2 + 1], 1) < 0) {
                        perror("dup2 write");
                        exit(EXIT_FAILURE);
                    }
                }

                for (int j = 0; j < 2 * (command_count - 1); j++) close(pipefds[j]);

                char *args[100];
                int argc = 0;
                char *trimmed = trim_whitespace(commands[i]);
                parse_arguments(trimmed, args, &argc);
                args[argc] = NULL;

                if (argc == 0) {
                    fprintf(stderr, "Error: comando vacío\n");
                    exit(1);
                }

                execvp(args[0], args);
                fprintf(stderr, "%s: command not found\n", args[0]);
                for (int j = 0; j < argc; j++) free(args[j]);
                exit(127);
            }
        }

        for (int i = 0; i < 2 * (command_count - 1); i++) close(pipefds[i]);
        for (int i = 0; i < command_count; i++) wait(NULL);
    }

    return 0;
}
