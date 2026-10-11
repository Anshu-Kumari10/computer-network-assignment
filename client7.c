// Roll: 24105142002 Service Type: C d = 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 30002
#define ROLL "24105142002"
#define SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server;
    socklen_t server_len = sizeof(server);
    char input[SIZE], message[SIZE], response[SIZE];

    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1],
                  &server.sin_addr) != 1) {
        printf("Invalid server IP address\n");
        close(sockfd);
        return 1;
    }

    printf("Connected to UDP server at %s:%d\n",
           argv[1], PORT);
    printf("Enter a string, or EXIT to quit.\n");

    while (1) {
        printf("\nEnter text: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            strcpy(input, "EXIT");
        }

        input[strcspn(input, "\n")] = '\0';

        snprintf(message, sizeof(message),
                 "%s#%s", ROLL, input);

        if (sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr *)&server,
                   server_len) < 0) {
            perror("Send failed");
            break;
        }

        memset(response, 0, sizeof(response));

        int n = recvfrom(sockfd, response, SIZE - 1, 0,
                         (struct sockaddr *)&server,
                         &server_len);

        if (n < 0) {
            perror("Receive failed");
            break;
        }

        response[n] = '\0';
        printf("Server response: %s\n", response);

        if (strcmp(input, "EXIT") == 0)
            break;
    }

    close(sockfd);
    return 0;
}