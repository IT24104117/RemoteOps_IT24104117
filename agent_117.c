#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9410
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    /* 1. Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    /* 2. Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* 3. Bind socket to port 9410 */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    /* 4. Start listening */
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("RemoteOps Agent listening on TCP port %d...\n", PORT);

    /* 5. Accept one client */
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Controller connected.\n");

    /* 6. Receive data */
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(client_fd,
                                  buffer,
                                  sizeof(buffer) - 1,
                                  0);

    if (bytes_received < 0)
    {
        perror("recv");
    }
    else
    {
        buffer[bytes_received] = '\0';

        printf("Received: %s", buffer);

        /* 7. Send response */
        const char *response = "OK Agent received your message.\n";

        if (send(client_fd,
                 response,
                 strlen(response),
                 0) < 0)
        {
            perror("send");
        }
    }

    /* 8. Close client connection */
    close(client_fd);

    /* 9. Close server socket */
    close(server_fd);

    return 0;
}
