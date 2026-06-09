#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

int safe_atoi(const char *s, int *out) {
    char *end;
    errno = 0;
    long val = strtol(s, &end, 10);
    if (*end != '\0' || val < INT_MIN || val > INT_MAX) return 0;
    *out = (int)val;
    return 1;
}

int safe_increment(int x) {
    if (x == INT_MAX) {
        ERROR("Error: overflow de entero al incrementar\n");
    }
    return x + 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        return 1;
    }

    int n, val, start;
    if (!safe_atoi(argv[1], &n) || n <= 0) ERROR("Error: argumento n inv\xE1lido\n");
    if (!safe_atoi(argv[2], &val)) ERROR("Error: argumento c inv\xE1lido\n");
    if (!safe_atoi(argv[3], &start) || start < 0 || start >= n) ERROR("Error: argumento s inv\xE1lido\n");

    int ring[n][2];
    for (int i = 0; i < n; i++) {
        if (pipe(ring[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            int read_fd = ring[i][0];
            int write_fd = ring[(i + 1) % n][1];

            for (int j = 0; j < n; j++) {
                if (ring[j][0] != read_fd) close(ring[j][0]);
                if (ring[j][1] != write_fd) close(ring[j][1]);
            }

            if (i == start) {
                if (write(write_fd, &val, sizeof(int)) != sizeof(int)) {
                    perror("write inicial");
                    exit(1);
                }

                int final;
                if (read(read_fd, &final, sizeof(int)) != sizeof(int)) {
                    perror("read final");
                    exit(1);
                }
                final = safe_increment(final);
                printf("Valor final recibido por el proceso %d: %d\n", start, final);
            } else {
                int x;
                if (read(read_fd, &x, sizeof(int)) != sizeof(int)) {
                    perror("read");
                    exit(1);
                }
                x = safe_increment(x);
                if (write(write_fd, &x, sizeof(int)) != sizeof(int)) {
                    perror("write");
                    exit(1);
                }
            }

            close(read_fd);
            close(write_fd);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        close(ring[i][0]);
        close(ring[i][1]);
    }

    int status, failed = 0;
    while (wait(&status) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) failed = 1;
    }

    return failed ? 1 : 0;
}