#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}

    /* Parsing of arguments */
    n = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start = atoi(argv[3]);
    /* TO COMPLETE */

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);

    int pipes[n+1][2];
    for (int i = 0; i < n + 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            int read_fd = pipes[i][0];
            int write_fd = pipes[i+1][1];

            for (int j = 0; j < n+1; j++) {
                if (pipes[j][0] != read_fd) close(pipes[j][0]);
                if (pipes[j][1] != write_fd) close(pipes[j][1]);
            }

            int val;
            read(read_fd, &val, sizeof(int));
            val++;
            write(write_fd, &val, sizeof(int));
            close(read_fd);
            close(write_fd);
            exit(0);
        }
    }

    // PADRE

    for (int i = 0; i < n + 1; i++) {
        if (i != start) close(pipes[i][1]);         // solo escribe al pipe[start]
        if (i != n) close(pipes[i][0]);             // solo lee del pipe[n]
    }

    write(pipes[start][1], buffer, sizeof(int));
    close(pipes[start][1]); // ya no escribe más

    read(pipes[n][0], buffer, sizeof(int));
    close(pipes[n][0]);

    printf("Resultado final recibido por el padre: %d\n", buffer[0]);
    while (waitpid(-1, &status, 0) > 0);

    return 0;
}