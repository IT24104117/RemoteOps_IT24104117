#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <stdarg.h>

#define PORT 9410
#define BUFFER_SIZE 1024

#define AUTH_TOKEN "OPS-4117"
#define SID "7114"

#define STORAGE_DIR "./agentfiles/IT24104117/"
#define MAX_FILE_SIZE (10 * 1024 * 1024)

#define LOG_FILE "RemoteOps_IT24104117.log"
void write_log(const char *format, ...);

volatile int monitor_running = 0;
pthread_t monitor_thread;
struct sockaddr_in monitor_addr;

int recv_line(int fd, char *buffer, size_t max_size)
{
    size_t i = 0;
    char ch;

    while (i < max_size - 1)
    {
        ssize_t n = recv(fd, &ch, 1, 0);

        if (n == 0)
        {
            return 0;   /* Connection closed */
        }

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (ch == '\n')
        {
            buffer[i] = '\0';
            return 1;
        }

        if (ch != '\r')
        {
            buffer[i++] = ch;
        }
    }

    buffer[i] = '\0';

    return 1;
}

/*
 * Receive exactly 'total' bytes.
 * TCP may deliver data in multiple chunks.
 */
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
        {
            return 0;
        }

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

void *monitor_function(void *arg)
{
    int client_fd = *(int *)arg;

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_fd < 0)
    {
        perror("UDP socket");
        return NULL;
    }

    while (monitor_running)
    {
        struct sysinfo info;

        if (sysinfo(&info) == 0)
        {
            double cpu_load =
                info.loads[0] / 65536.0;

            unsigned long mem_used_mb =
                (info.totalram - info.freeram)
                * info.mem_unit
                / (1024 * 1024);

            char message[BUFFER_SIZE];

            snprintf(message,
                     sizeof(message),
                     "OK SYSINFO %.2f %lu %ld SID:%s\n",
                     cpu_load,
                     mem_used_mb,
                     info.uptime,
                     SID);

            sendto(udp_fd,
                   message,
                   strlen(message),
                   0,
                   (struct sockaddr *)&monitor_addr,
                   sizeof(monitor_addr));

            printf("Monitor UDP sent: %s", message);
        }

        sleep(5);

        /*
         * Check whether TCP connection is still active.
         */
        if (client_fd < 0)
            break;
    }

    close(udp_fd);

    return NULL;
}

void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    printf("Controller connected.\n");
    write_log("Controller connected");

    /* Receive AUTH command */
    memset(buffer, 0, sizeof(buffer));

int result = recv_line(client_fd,
                       buffer,
                       sizeof(buffer));

if (result <= 0)
{
    close(client_fd);
    return NULL;
}

    printf("Received: %s\n", buffer);
    write_log("Command: %s", buffer);

    /* Authentication */
    if (strcmp(buffer, "AUTH " AUTH_TOKEN) != 0)
    {
        const char *response =
            "ERR 001 AUTH_FAILED SID:" SID "\n";

        send(client_fd, response, strlen(response), 0);

        printf("Authentication failed.\n");
        write_log("Authentication failed");

        close(client_fd);
        return NULL;
    }

    /* Authentication successful */
    const char *auth_response =
        "OK AUTHENTICATED SID:" SID "\n";

    if (send(client_fd,
             auth_response,
             strlen(auth_response),
             0) < 0)
    {
        perror("send");
        close(client_fd);
        return NULL;
    }

    printf("Authentication successful.\n");
    write_log("Authentication successful"); 

    /*
     * Receive commands after successful authentication
     */
while (1)
{
    int result = recv_line(client_fd,
                           buffer,
                           sizeof(buffer));

    if (result <= 0)
    {
        break;
    }

    printf("Command received: %s\n", buffer);
    write_log("Command: %s", buffer);

/* SYSINFO command */
if (strcmp(buffer, "SYSINFO") == 0)
{
    struct sysinfo info;

    if (sysinfo(&info) == 0)
    {
        char response[BUFFER_SIZE];

        unsigned long total_mb =
            info.totalram / 1024 / 1024;

        unsigned long free_mb =
            info.freeram / 1024 / 1024;

        unsigned long used_mb =
            total_mb - free_mb;

        double cpu_load =
            (double)info.loads[0] / 65536.0;

        snprintf(response,
                 sizeof(response),
                 "OK SYSINFO %.2f %lu %ld SID:%s\n",
                 cpu_load,
                 used_mb,
                 info.uptime,
                 SID);

        send(client_fd,
             response,
             strlen(response),
             0);
    }
    else
    {
        const char *response =
            "ERR 003 SYSINFO_FAILED SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);
    }
}

/* LISTPROC command */
else if (strcmp(buffer, "LISTPROC") == 0)
{
    FILE *processes;
    char line[256];
    char response[BUFFER_SIZE];

    response[0] = '\0';

    processes = popen("ps -eo pid=,comm= --no-headers", "r");

    if (processes == NULL)
    {
        const char *error =
            "ERR 003 LISTPROC_FAILED SID:" SID "\n";

        send(client_fd,
             error,
             strlen(error),
             0);
    }
    else
    {
        strcat(response, "OK PROCS ");

        while (fgets(line, sizeof(line), processes) != NULL)
        {
            line[strcspn(line, "\r\n")] = '\0';

            if (strlen(response) + strlen(line) + 2 <
                sizeof(response) - 30)
            {
                strcat(response, line);
                strcat(response, ",");
            }
        }

        pclose(processes);

        /* Remove final comma */
        size_t len = strlen(response);

        if (len > 0 && response[len - 1] == ',')
        {
            response[len - 1] = '\0';
        }

        strcat(response, " SID:" SID "\n");

        send(client_fd,
             response,
             strlen(response),
             0);
    }
}

/* EXEC command */
else if (strncmp(buffer, "EXEC ", 5) == 0)
{
    char command[32];
    char output[BUFFER_SIZE];
    FILE *fp;

    strcpy(command, buffer + 5);

    /* Check whitelist */
    if (strcmp(command, "DATE") != 0 &&
        strcmp(command, "UPTIME") != 0 &&
        strcmp(command, "DISKFREE") != 0 &&
        strcmp(command, "HOSTNAME") != 0 &&
        strcmp(command, "WHOAMI") != 0)
    {
        const char *response =
            "ERR 002 COMMAND_NOT_ALLOWED SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    /*
     * Only fixed commands are executed.
     * No arbitrary user command is passed to the shell.
     */
    if (strcmp(command, "DATE") == 0)
    {
        fp = popen("date", "r");
    }
    else if (strcmp(command, "UPTIME") == 0)
    {
        fp = popen("uptime", "r");
    }
    else if (strcmp(command, "DISKFREE") == 0)
    {
        fp = popen("df -h /", "r");
    }
    else if (strcmp(command, "HOSTNAME") == 0)
    {
        fp = popen("hostname", "r");
    }
    else
    {
        fp = popen("whoami", "r");
    }

    if (fp == NULL)
    {
        const char *response =
            "ERR 003 EXEC_FAILED SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    output[0] = '\0';

    while (fgets(output + strlen(output),
                 sizeof(output) - strlen(output),
                 fp) != NULL)
    {
        /* Keep reading output */
    }

    pclose(fp);

    /* Convert multi-line command output to one protocol line */
    for (size_t i = 0; output[i] != '\0'; i++)
    {
        if (output[i] == '\n' || output[i] == '\r')
        {
            output[i] = ' ';
        }
    }

char response[BUFFER_SIZE];

snprintf(response,
         sizeof(response),
         "OK EXEC_RESULT %.900s SID:%s\n",
         output,
         SID);

    send(client_fd,
         response,
         strlen(response),
         0);
}
 
/* PUT command */
else if (strncmp(buffer, "PUT ", 4) == 0)
{
    write_log("Command: %s", buffer);

    char filename[256];
    unsigned long long filesize;
    char filepath[512];

    if (sscanf(buffer + 4,
               "%255s %llu",
               filename,
               &filesize) != 2)
    {
        const char *response =
            "ERR 004 INVALID_PUT SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    /* Prevent path traversal */
    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\') != NULL)
    {
        const char *response =
            "ERR 004 INVALID_FILENAME SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    if (filesize > MAX_FILE_SIZE)
    {
        const char *response =
            "ERR 004 FILE_TOO_LARGE SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    snprintf(filepath,
             sizeof(filepath),
             "%s%s",
             STORAGE_DIR,
             filename);

    FILE *fp = fopen(filepath, "wb");

    if (fp == NULL)
    {
        const char *response =
            "ERR 004 FILE_OPEN_FAILED SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    char file_buffer[4096];
    unsigned long long remaining = filesize;
    int transfer_ok = 1;

    while (remaining > 0)
    {
        size_t chunk_size =
            remaining > sizeof(file_buffer)
            ? sizeof(file_buffer)
            : (size_t)remaining;

        int result = recv_all(client_fd,
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

    if (!transfer_ok)
    {
        remove(filepath);

        const char *response =
            "ERR 004 FILE_RECEIVE_FAILED SID:" SID "\n";

        send(client_fd,
             response,
             strlen(response),
             0);

        continue;
    }

    char response[BUFFER_SIZE];

    snprintf(response,
             sizeof(response),
             "OK FILE_RECEIVED %s SID:%s\n",
             filename,
             SID);

    send(client_fd,
         response,
         strlen(response),
         0);
    write_log("File received: %s (%llu bytes)",
          filename,
          filesize);

}

/* GET command */
else if (strncmp(buffer, "GET ", 4) == 0)
{

    char filename[256];
    char filepath[512];

    if (sscanf(buffer + 4, "%255s", filename) != 1)
    {
        const char *response =
            "ERR 005 INVALID_GET SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    /* Prevent path traversal */
    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\') != NULL)
    {
        const char *response =
            "ERR 005 INVALID_FILENAME SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    snprintf(filepath,
             sizeof(filepath),
             "%s%s",
             STORAGE_DIR,
             filename);

    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL)
    {
        const char *response =
            "ERR 005 FILE_NOT_FOUND SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    /* Find file size */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);

        const char *response =
            "ERR 005 FILE_READ_FAILED SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    long file_size = ftell(fp);

    if (file_size < 0)
    {
        fclose(fp);

        const char *response =
            "ERR 005 FILE_READ_FAILED SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    rewind(fp);

    /* Send GET response header */
    char response[BUFFER_SIZE];

    snprintf(response,
             sizeof(response),
             "OK FILE_SEND %s %ld SID:%s\n",
             filename,
             file_size,
             SID);

    if (send_all(client_fd,
                 response,
                 strlen(response)) != 1)
    {
        fclose(fp);
        continue;
    }

    /* Send exact file bytes */
    char file_buffer[4096];
    size_t bytes_read;
    int send_success = 1;

    while ((bytes_read = fread(file_buffer,
                               1,
                               sizeof(file_buffer),
                               fp)) > 0)
    {
        if (send_all(client_fd,
                     file_buffer,
                     bytes_read) != 1)
        { 
            send_success = 0;
            break;
        }
    }
    if (send_success)
{
    write_log("File sent: %s (%ld bytes)",
              filename,
              file_size);
}

    fclose(fp);
}

/* MONITOR START */
else if (strncmp(buffer, "MONITOR START ", 14) == 0)
{
    int udp_port;

    if (sscanf(buffer + 14, "%d", &udp_port) != 1 ||
        udp_port < 1 ||
        udp_port > 65535)
    {
        const char *response =
            "ERR 003 INVALID_MONITOR_PORT SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    if (monitor_running)
    {
        const char *response =
            "ERR 003 MONITOR_ALREADY_RUNNING SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    /*
     * Get Controller IP address from TCP connection.
     */
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);

    if (getpeername(client_fd,
                    (struct sockaddr *)&peer_addr,
                    &peer_len) < 0)
    {
        perror("getpeername");

        const char *response =
            "ERR 003 MONITOR_ADDRESS_FAILED SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    memset(&monitor_addr, 0, sizeof(monitor_addr));

    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(udp_port);
    monitor_addr.sin_addr = peer_addr.sin_addr;

    monitor_running = 1;

    if (pthread_create(&monitor_thread,
                       NULL,
                       monitor_function,
                       &client_fd) != 0)
    {
        monitor_running = 0;

        const char *response =
            "ERR 003 MONITOR_START_FAILED SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    pthread_detach(monitor_thread);

    const char *response =
        "OK MONITOR_STARTED SID:" SID "\n";

    send_all(client_fd,
             response,
             strlen(response));
}

/* MONITOR STOP */
else if (strcmp(buffer, "MONITOR STOP") == 0)
{
    if (!monitor_running)
    {
        const char *response =
            "ERR 003 MONITOR_NOT_RUNNING SID:" SID "\n";

        send_all(client_fd,
                 response,
                 strlen(response));

        continue;
    }

    monitor_running = 0;

    const char *response =
        "OK MONITOR_STOPPED SID:" SID "\n";

    send_all(client_fd,
             response,
             strlen(response));
}

       /* QUIT command */
        else if (strcmp(buffer, "QUIT") == 0)
        {

            write_log("Controller requested QUIT");
            const char *response = "OK BYE SID:" SID "\n";

            send(client_fd,
                 response,
                 strlen(response),
                 0);

            break;
        }

        /* Unknown command */
        else
        {
            const char *response =
               "ERR 002 UNKNOWN_COMMAND SID:" SID "\n";
            send(client_fd,
                 response,
                 strlen(response),
                 0);
        }
    }

    printf("Controller disconnected.\n");
    write_log("Controller disconnected");

    close(client_fd);

    return NULL;
}

void write_log(const char *format, ...)
{
    FILE *log_file;
    time_t now;
    struct tm *time_info;
    char timestamp[64];

    now = time(NULL);
    time_info = localtime(&now);

    strftime(timestamp,
             sizeof(timestamp),
             "%Y-%m-%d %H:%M:%S",
             time_info);

    log_file = fopen(LOG_FILE, "a");

    if (log_file == NULL)
    {
        perror("Log file");
        return;
    }

    fprintf(log_file, "[%s] ", timestamp);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");

    fclose(log_file);
}

int main(void)
{
    int server_fd;

    struct sockaddr_in server_addr;

    /* Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    /* Allow address reuse */
    int opt = 1;

    if (setsockopt(server_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Bind */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    /* Listen */
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("RemoteOps Agent listening on TCP port %d...\n",
           PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        /* Allocate client socket for thread */
        int *client_socket = malloc(sizeof(int));

        if (client_socket == NULL)
        {
            perror("malloc");
            close(client_fd);
            continue;
        }

        *client_socket = client_fd;

        pthread_t thread;

        if (pthread_create(&thread,
                           NULL,
                           handle_client,
                           client_socket) != 0)
        {
            perror("pthread_create");
            free(client_socket);
            close(client_fd);
            continue;
        }

        /* Automatically clean up thread resources */
        pthread_detach(thread);
    }

    close(server_fd);

    return 0;
}
