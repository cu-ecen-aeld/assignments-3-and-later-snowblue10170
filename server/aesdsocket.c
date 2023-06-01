#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 9000

int server_fd, client_fd;
FILE *file;
struct sockaddr_in address;
int addrlen = sizeof(address);
char *data;
long totalSize = 0;

void sig_handler(int signum) {
    if (client_fd)
        close(client_fd);
    if (server_fd)
        close(server_fd);
    if (file)
        fclose(file);
    remove("/var/tmp/aesdsocketdata");
    if (data)
        free(data);
    syslog(LOG_INFO, "Caught signal, exiting");
    closelog();
    exit(0);
}

void start_server() {
    char buffer[1024] = {0};
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        syslog(LOG_ERR, "socket failed");
        exit(-1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "bind failed");
        exit(-1);
    }

    if (listen(server_fd, 3) < 0) {
        syslog(LOG_ERR, "listen failed");
        exit(-1);
    }

    while (1) {
        syslog(LOG_INFO, "Waiting for a connection");
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            syslog(LOG_ERR, "accept failed");
            continue;
        }

        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(address.sin_addr));

        file = fopen("/var/tmp/aesdsocketdata", "a+");
        if (!file) {
            syslog(LOG_ERR, "fopen failed");
            close(client_fd);
            continue;
        }

        int valread;
while ((valread = recv(client_fd, buffer, 1024, 0)) > 0) {

    for (int i = 0; i < valread; i++)
            {
                if (buffer[i] == '\n')
                {
                    fputs("\n", file);
                }
                else
                {
                    fputc(buffer[i], file);
                }
            }
            if (valread < sizeof(buffer))
                break;
}
if (valread < 0) {
    syslog(LOG_ERR, "recv failed");
}
fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *response_data = malloc(file_size);
        if (response_data == NULL)
        {
            fclose(file);
            syslog(LOG_ERR, "Failed to allocate memory for response");
            close(client_fd);
            continue;
        }

        fread(response_data, 1, file_size, file);
        send(client_fd, response_data, file_size, 0);
        free(response_data);

        fclose(file);
        file = NULL;
        
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(address.sin_addr));
    }
    close(client_fd);
        client_fd = 0;
}

int main(int argc, char const *argv[]) {
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    int daemon_flag = 0;
    int opt;

    while((opt = getopt(argc, (char * const*)argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_flag = 1;
                fprintf(stderr, "daemon_flag = 1");
                break;
            default:
                fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (daemon_flag) {
        pid_t pid = fork();

        if (pid < 0)
            exit(EXIT_FAILURE);

        if (pid > 0)
            exit(EXIT_SUCCESS);

    }

    start_server();

    return 0;
}


