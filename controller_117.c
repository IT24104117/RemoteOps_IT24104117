#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9410
#define BUFFER_SIZE 4096

int send_all(int fd, const void *buffer, size_t total)
{
    size_t sent = 0;
    const char *ptr = buffer;

    while (sent < total)
    {
        ssize_t n = send(fd,
                         ptr + sent,
                         total - sent,
                         0);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("send");
            return -1;
        }

        if (n == 0)
            return 0;

        sent += (size_t)n;
    }

    return 1;
}

int recv_all(int fd, void *buffer, size_t total)
{
    size_t received = 0;
    char *ptr = buffer;

    while (received < total)
    {
        ssize_t n = recv(fd,
                         ptr + received,
                         total - received,
                         0);

        if (n == 0)
            return 0;

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("recv");
            return -1;
        }

        received += (size_t)n;
    }

    return 1;
}

void *udp_monitor_listener(void *arg)
{
    int udp_port = *(int *)arg;

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_fd < 0)
    {
        perror("UDP socket");
        return NULL;
    }

    struct sockaddr_in local_addr;

    memset(&local_addr, 0, sizeof(local_addr));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(udp_port);

    if (bind(udp_fd,
             (struct sockaddr *)&local_addr,
             sizeof(local_addr)) < 0)
    {
        perror("UDP bind");
        close(udp_fd);
        return NULL;
    }

    printf("UDP Monitor listening on port %d...\n", udp_port);

    while (1)
    {
        char buffer[BUFFER_SIZE];

        ssize_t bytes_received =
            recvfrom(udp_fd,
                     buffer,
                     sizeof(buffer) - 1,
                     0,
                     NULL,
                     NULL);

        if (bytes_received < 0)
        {
            perror("recvfrom");
            break;
        }

        buffer[bytes_received] = '\0';

        printf("Monitor UDP: %s", buffer);
        fflush(stdout);
    }

    close(udp_fd);

    return NULL;
}

int main(void)
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    /* Create TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    /* Server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    /* Connect to Agent */
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("Connected to RemoteOps Agent.\n");
int udp_port = 9500;
pthread_t udp_thread;

if (pthread_create(&udp_thread,
                   NULL,
                   udp_monitor_listener,
                   &udp_port) != 0)
{
    perror("pthread_create");
    close(sockfd);
    return 1;
}

pthread_detach(udp_thread);


    /* AUTH */
    const char *auth = "AUTH OPS-4117\n";

    send(sockfd, auth, strlen(auth), 0);

    printf("Sent: AUTH OPS-4117\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received <= 0)
    {
        printf("Agent disconnected.\n");
        close(sockfd);
        return 1;
    }

    buffer[bytes_received] = '\0';

    printf("Agent response: %s", buffer);

    /* SYSINFO */
    const char *sysinfo = "SYSINFO\n";

    send(sockfd, sysinfo, strlen(sysinfo), 0);

    printf("Sent: SYSINFO\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }

    /* LISTPROC */
    const char *listproc = "LISTPROC\n";

    send(sockfd, listproc, strlen(listproc), 0);

    printf("Sent: LISTPROC\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }

/* EXEC DATE */
const char *exec_date = "EXEC DATE\n";

send(sockfd, exec_date, strlen(exec_date), 0);

printf("Sent: EXEC DATE\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* EXEC UPTIME */
const char *exec_uptime = "EXEC UPTIME\n";

send(sockfd, exec_uptime, strlen(exec_uptime), 0);

printf("Sent: EXEC UPTIME\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* EXEC DISKFREE */
const char *exec_diskfree = "EXEC DISKFREE\n";

send(sockfd, exec_diskfree, strlen(exec_diskfree), 0);

printf("Sent: EXEC DISKFREE\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* EXEC HOSTNAME */
const char *exec_hostname = "EXEC HOSTNAME\n";

send(sockfd, exec_hostname, strlen(exec_hostname), 0);

printf("Sent: EXEC HOSTNAME\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* EXEC WHOAMI */
const char *exec_whoami = "EXEC WHOAMI\n";

send(sockfd, exec_whoami, strlen(exec_whoami), 0);

printf("Sent: EXEC WHOAMI\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* Invalid EXEC command */
const char *exec_invalid = "EXEC LS\n";

send(sockfd, exec_invalid, strlen(exec_invalid), 0);

printf("Sent: EXEC LS\n");

bytes_received = recv(sockfd,
                      buffer,
                      sizeof(buffer) - 1,
                      0);

if (bytes_received > 0)
{
    buffer[bytes_received] = '\0';
    printf("Agent response: %s", buffer);
}

/* PUT */
{
    const char *filename = "test.txt";
    const char *file_data =
        "Hello from RemoteOps Controller!\n"
        "This is a PUT test file.\n";

    unsigned long long filesize = strlen(file_data);

    char put_command[BUFFER_SIZE];

    snprintf(put_command,
             sizeof(put_command),
             "PUT %s %llu\n",
             filename,
             filesize);

    send_all(sockfd,
             put_command,
             strlen(put_command));

    printf("Sent: PUT %s %llu bytes\n",
           filename,
           filesize);

    send_all(sockfd,
             file_data,
             filesize);

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }
}

/* GET */
{
    const char *filename = "test.txt";
    char get_command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    snprintf(get_command,
             sizeof(get_command),
             "GET %s\n",
             filename);

    send_all(sockfd,
             get_command,
             strlen(get_command));

    printf("Sent: GET %s\n", filename);

    /*
     * Receive GET response header.
     */
    bytes_received = recv(sockfd,
                          response,
                          sizeof(response) - 1,
                          0);

    if (bytes_received <= 0)
    {
        printf("Agent disconnected during GET.\n");
    }
    else
    {
        response[bytes_received] = '\0';

        printf("Agent response: %s", response);

        /*
         * Check successful FILE_SEND response.
         */
        unsigned long long filesize;
        char received_filename[256];

        if (sscanf(response,
                   "OK FILE_SEND %255s %llu SID:7114",
                   received_filename,
                   &filesize) == 2)
        {
            FILE *fp = fopen("downloaded_test.txt", "wb");

            if (fp == NULL)
            {
                perror("fopen");
            }
            else
            {
                char file_buffer[4096];
                unsigned long long remaining = filesize;
                int transfer_ok = 1;

                while (remaining > 0)
                {
                    size_t chunk_size =
                        remaining > sizeof(file_buffer)
                        ? sizeof(file_buffer)
                        : (size_t)remaining;

                    int result = recv_all(sockfd,
                                          file_buffer,
                                          chunk_size);

                    if (result != 1)
                    {
                        transfer_ok = 0;
                        break;
                    }

                    if (fwrite(file_buffer,
                               1,
                               chunk_size,
                               fp) != chunk_size)
                    {
                        transfer_ok = 0;
                        break;
                    }

                    remaining -= chunk_size;
                }

                fclose(fp);

                if (transfer_ok)
                {
                    printf("GET successful: %s (%llu bytes) saved as downloaded_test.txt\n",
                           received_filename,
                           filesize);
                }
                else
                {
                    printf("GET failed while receiving file.\n");
                    remove("downloaded_test.txt");
                }
            }
        }
    }
}

/* MONITOR START */
{
    const char *monitor_start = "MONITOR START 9500\n";

    send_all(sockfd,
             monitor_start,
             strlen(monitor_start));

    printf("Sent: MONITOR START 9500\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }
}

sleep(12);


/* MONITOR STOP */
{
    const char *monitor_stop = "MONITOR STOP\n";

    send_all(sockfd,
             monitor_stop,
             strlen(monitor_stop));

    printf("Sent: MONITOR STOP\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }
}


    /* QUIT */
    const char *quit = "QUIT\n";

    send(sockfd, quit, strlen(quit), 0);

    printf("Sent: QUIT\n");

    bytes_received = recv(sockfd,
                          buffer,
                          sizeof(buffer) - 1,
                          0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        printf("Agent response: %s", buffer);
    }

    close(sockfd);

    return 0;
}
