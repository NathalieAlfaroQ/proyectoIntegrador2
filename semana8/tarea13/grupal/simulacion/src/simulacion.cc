#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

// Function prototypes
void servidor(int, int);
void cliente(int, int);
void tenedor(int, int, int, int);

int main(int argc, char* argv[]) {
    int client_to_fork[2];
    int fork_to_server[2];
    int server_to_fork[2];
    int fork_to_client[2];

    if (pipe(client_to_fork) < 0 || pipe(fork_to_server) < 0
        || pipe(server_to_fork) < 0 || pipe(fork_to_client) < 0)
        exit(1);

    pid_t server_pid = fork();
    if (server_pid == 0) {
        // Server process
        close(client_to_fork[0]);
        close(client_to_fork[1]);
        close(fork_to_server[1]);
        close(fork_to_client[0]);
        close(fork_to_client[1]);
        // Correct: read from fork_to_server[0], write to server_to_fork[1]
        servidor(fork_to_server[0], server_to_fork[1]);
        close(fork_to_server[0]);
        close(server_to_fork[1]);
        exit(0);
    }

    pid_t fork_pid = fork();
    if (fork_pid == 0) {
        // Fork process
        close(client_to_fork[1]); // only read from client
        close(fork_to_server[0]); // only write to server
        close(server_to_fork[1]); // only read from server
        close(fork_to_client[0]); // only write to client
        tenedor(client_to_fork[0], fork_to_server[1], server_to_fork[0], fork_to_client[1]);
        close(client_to_fork[0]);
        close(fork_to_server[1]);
        close(server_to_fork[0]);
        close(fork_to_client[1]);
        exit(0);
    }

    // Client process (parent)
    close(client_to_fork[0]); // only write to fork
    close(fork_to_server[0]);
    close(fork_to_server[1]);
    close(server_to_fork[0]);
    close(server_to_fork[1]);
    close(fork_to_client[1]); // only read from fork

    cliente(fork_to_client[0], client_to_fork[1]);

    close(client_to_fork[1]);
    close(fork_to_client[0]);

    waitpid(server_pid, NULL, 0);
    waitpid(fork_pid, NULL, 0);

    return EXIT_SUCCESS;
}
