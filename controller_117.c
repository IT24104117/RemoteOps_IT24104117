#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid IP address");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("Connected to Agent at %s:%d\n", ip, port);

    char buffer[1024];

    printf("Enter command: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        close(sockfd);
        return 1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    if (send(sockfd, buffer, strlen(buffer), 0) < 0)
    {
        perror("send");
        close(sockfd);
        return 1;
    }

    int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    if (n < 0)
    {
        perror("recv");
        close(sockfd);
        return 1;
    }

    buffer[n] = '\0';

    printf("Agent response: %s\n", buffer);

    close(sockfd);

    return 0;
}
